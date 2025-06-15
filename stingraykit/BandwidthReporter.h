#ifndef STINGRAYKIT_BANDWIDTHREPORTER_H
#define STINGRAYKIT_BANDWIDTHREPORTER_H

// Copyright (c) 2011 - 2025, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/executor/ITimer.h>
#include <stingraykit/io/IDataSource.h>
#include <stingraykit/time/ElapsedTime.h>

namespace stingray
{

	class BandwidthReporter final : public virtual IDataSource
	{
	private:
		IDataSourcePtr			_source;

		Mutex					_mutex;
		u64						_dataTotal;
		u64						_dataSinceLastReport;
		ElapsedTime				_timeSinceLastReport;

		ITimerPtr				_timer;
		Token					_connection;

	public:
		BandwidthReporter(const IDataSourcePtr& source, const std::string& timerName);

		void Read(IDataConsumer& consumer, const ICancellationToken& token) override;

	private:
		void Report();

		size_t DoPush(IDataConsumer& consumer, ConstByteData data, const ICancellationToken& token);
		void BytesProcessed(size_t bytesCount);
	};

}

#endif
