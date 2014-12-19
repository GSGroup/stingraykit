#include <stingray/toolkit/diagnostics/SystemProfiler.h>

#include <time.h>

#include <stingray/toolkit/log/Logger.h>
#include <stingray/toolkit/SystemException.h>


namespace stingray
{

#ifdef PLATFORM_POSIX
	class SystemProfiler::Impl
	{
	public:
		s64 GetMicroseconds() const
		{
			struct timespec ts = {};
			if (clock_gettime(CLOCK_MONOTONIC, &ts) == -1)
				throw SystemException("clock_gettime");
			return (s64)ts.tv_sec * 1000000 + ts.tv_nsec / 1000;
		}
	};

#else

	class SystemProfiler::Impl
	{
	public:
		s64 GetMicroseconds() const { return 0; }
	};

#endif


	SystemProfiler::SystemProfiler(const std::string& message, s64 thresholdMs, s64 criticalMs)
		: _impl(new Impl()), _message(message), _thresholdMs(thresholdMs), _criticalMs(criticalMs)
	{ _start = _impl->GetMicroseconds(); }


	SystemProfiler::~SystemProfiler()
	{
		s64 elapsed_ms = (_impl->GetMicroseconds() - _start) / 1000;

		if (elapsed_ms < _thresholdMs)
			return;

		LogLevel ll = (_criticalMs >= _thresholdMs && elapsed_ms > _criticalMs) ? LogLevel::Warning : LogLevel::Info;
		Logger::Stream(ll) << _message << ": " << elapsed_ms << " ms";
	}

}
