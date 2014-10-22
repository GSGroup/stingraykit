#ifndef STINGRAY_TOOLKIT_BANDWIDTHREPORTER_H
#define STINGRAY_TOOLKIT_BANDWIDTHREPORTER_H


#include <stingray/timer/Timer.h>

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
		TimerConnectionHolder	_connection;

	public:
		BandwidthReporter(const IDataSourcePtr& source, const std::string& timerName) :
			_source(source), _dataTotal(0), _dataSinceLastReport(0), _timer(timerName)
		{ _connection = _timer.SetTimer(ReportBandwidthTimeout, bind(&BandwidthReporter::Report, this)); }

		virtual ~BandwidthReporter()
		{ _connection.Disconnect(); }

		virtual void Read(IDataConsumer& consumer, const ICancellationToken& token)
		{ _source->ReadToFunction(bind(&BandwidthReporter::DoPush, this, ref(consumer), _1, _2), bind(&IDataConsumer::EndOfData, ref(consumer)), token); }

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
