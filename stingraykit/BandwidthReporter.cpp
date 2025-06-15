// Copyright (c) 2011 - 2025, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/BandwidthReporter.h>

namespace stingray
{

	namespace
	{

		const s64 ReportBandwidthTimeout = 10000;

	}


	BandwidthReporter::BandwidthReporter(const IDataSourcePtr& source, const std::string& timerName)
		:	_source(STINGRAYKIT_REQUIRE_NOT_NULL(source)),
			_dataTotal(0),
			_dataSinceLastReport(0),
			_timer(timerName)
	{ _connection = _timer.SetTimer(TimeDuration(ReportBandwidthTimeout), Bind(&BandwidthReporter::Report, this)); }


	void BandwidthReporter::Read(IDataConsumer& consumer, const ICancellationToken& token)
	{ _source->ReadToFunction(Bind(&BandwidthReporter::DoPush, this, wrap_ref(consumer), _1, _2), Bind(&IDataConsumer::EndOfData, wrap_ref(consumer), _1), token); }


	void BandwidthReporter::Report()
	{
		MutexLock l(_mutex);

		const u64 speedInKbytes = (_dataSinceLastReport * 1000 / _timeSinceLastReport.ElapsedMilliseconds()) / 1024;

		Logger::Info() << "Data: " << _dataSinceLastReport << " total: " << _dataTotal << " avg speed: " << speedInKbytes << " KB/s";

		_dataSinceLastReport = 0;
		_timeSinceLastReport.Restart();
	}


	size_t BandwidthReporter::DoPush(IDataConsumer& consumer, ConstByteData data, const ICancellationToken& token)
	{
		size_t size = consumer.Process(data, token);
		BytesProcessed(size);
		return size;
	}


	void BandwidthReporter::BytesProcessed(size_t bytesCount)
	{
		MutexLock l(_mutex);
		_dataSinceLastReport += bytesCount;
		_dataTotal += bytesCount;
	}

}
