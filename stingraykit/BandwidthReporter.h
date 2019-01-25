#ifndef STINGRAYKIT_BANDWIDTHREPORTER_H
#define STINGRAYKIT_BANDWIDTHREPORTER_H

// Copyright (c) 2011 - 2019, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/io/IDataSource.h>
#include <stingraykit/timer/Timer.h>

namespace stingray
{

	class BandwidthReporter : public virtual IDataSource
	{
		static const s64 ReportBandwidthTimeout = 10000;

	private:
		IDataSourcePtr			_source;

		Mutex					_mutex;
		u64						_dataTotal;
		u64						_dataSinceLastReport;
		ElapsedTime				_timeSinceLastReport;

		Timer					_timer;
		Token					_connection;

	public:
		BandwidthReporter(const IDataSourcePtr& source, const std::string& timerName)
			:	_source(STINGRAYKIT_REQUIRE_NOT_NULL(source)),
				_dataTotal(0),
				_dataSinceLastReport(0),
				_timer(timerName)
		{ _connection = _timer.SetTimer(TimeDuration(ReportBandwidthTimeout), bind(&BandwidthReporter::Report, this)); }

		virtual void Read(IDataConsumer& consumer, const ICancellationToken& token)
		{ _source->ReadToFunction(bind(&BandwidthReporter::DoPush, this, ref(consumer), _1, _2), bind(&IDataConsumer::EndOfData, ref(consumer), _1), token); }

	private:
		void Report()
		{
			MutexLock l(_mutex);
			u64 speed_in_bytes = _dataSinceLastReport * 1000 / _timeSinceLastReport.ElapsedMilliseconds();
			Logger::Info() << "Data: " << _dataSinceLastReport << " total: " << _dataTotal << " avg speed: " << speed_in_bytes / 1024 << "." << speed_in_bytes % 1024 << " KB/s";

			_dataSinceLastReport = 0;
			_timeSinceLastReport.Restart();
		}

		size_t DoPush(IDataConsumer& consumer, ConstByteData data, const ICancellationToken& token)
		{
			size_t size = consumer.Process(data, token);
			BytesProcessed(size);
			return size;
		}

		void BytesProcessed(size_t bytesCount)
		{
			MutexLock l(_mutex);
			_dataSinceLastReport += bytesCount;
			_dataTotal += bytesCount;
		}
	};

}

#endif
