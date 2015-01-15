#ifndef STINGRAYKIT_DIAGNOSTICS_PROFILER_H
#define STINGRAYKIT_DIAGNOSTICS_PROFILER_H

#include <limits>

#include <stingraykit/log/Logger.h>
#include <stingraykit/time/ElapsedTime.h>
#include <stingraykit/string/StringUtils.h>


namespace stingray
{

	/**
	 * @addtogroup toolkit_profiling
	 * @{
	 */

	class Profiler
	{
	private:
		Detail::NamedLoggerAccessor	_logger;
		ElapsedTime					_et;
		std::string					_message;
		u64							_thresholdMs;
		u64							_criticalMs;

	public:
		explicit Profiler(const std::string& message, u64 thresholdMs = 0, u64 criticalMs = std::numeric_limits<size_t>::max())
			: _message(message), _thresholdMs(thresholdMs), _criticalMs(criticalMs)
		{ }

		explicit Profiler(const Detail::NamedLoggerAccessor& logger, const std::string& message, u64 thresholdMs = 0, u64 criticalMs = std::numeric_limits<size_t>::max())
			: _logger(logger), _message(message), _thresholdMs(thresholdMs), _criticalMs(criticalMs)
		{ }

		~Profiler()
		{
			try
			{
				u64 ms = _et.ElapsedMilliseconds();
				if (ms > _thresholdMs)
				{
					LogLevel ll = (ms > _criticalMs) ? LogLevel::Warning : LogLevel::Info;
					_logger.Stream(ll) << ms << " milliseconds - " << _message;
				}
			}
			catch (const std::exception& ex)
			{
				try { Logger::Warning() << "Exception in " << STINGRAYKIT_WHERE << ": " << diagnostic_information(ex); }
				catch (const std::exception& ex) { }
			}
		}
	};


#define STINGRAYKIT_PROFILER(Milliseconds_, Message_) \
	DETAIL_DECLARE_STATIC_LOGGER_ACCESSOR; \
	Profiler STINGRAYKIT_CAT(detail_profiler, __LINE__)(STINGRAYKIT_STATIC_LOGGER, StringBuilder() % STINGRAYKIT_WHERE % ": " % (Message_), Milliseconds_);

#define STINGRAYKIT_PROFILER_NO_WHERE(Milliseconds_, Message_) \
	DETAIL_DECLARE_STATIC_LOGGER_ACCESSOR; \
	Profiler STINGRAYKIT_CAT(detail_profiler, __LINE__)(STINGRAYKIT_STATIC_LOGGER, (Message_), Milliseconds_);

#define STINGRAYKIT_PROFILER_CRITICAL_TIME(Milliseconds_, Message_) \
	DETAIL_DECLARE_STATIC_LOGGER_ACCESSOR; \
	Profiler STINGRAYKIT_CAT(detail_profiler, __LINE__)(STINGRAYKIT_STATIC_LOGGER, StringBuilder() % STINGRAYKIT_WHERE % ": " % (Message_), Milliseconds_, Milliseconds_);

#define STINGRAYKIT_PROFILE(Milliseconds_, Call_) \
	do { \
		STINGRAYKIT_PROFILER(Milliseconds_, #Call_); \
		Call_; \
	} while (false)

#define STINGRAYKIT_PROFILE_CRITICAL_TIME(Milliseconds_, Call_) \
	do { \
		STINGRAYKIT_PROFILER_CRITICAL_TIME(Milliseconds_, #Call_); \
		Call_; \
	} while (false)

	/** @} */

}


#endif
