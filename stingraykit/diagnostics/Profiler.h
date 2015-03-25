#ifndef STINGRAYKIT_DIAGNOSTICS_PROFILER_H
#define STINGRAYKIT_DIAGNOSTICS_PROFILER_H

// Copyright (c) 2011 - 2015, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <limits>

#include <stingraykit/log/Logger.h>
#include <stingraykit/time/ElapsedTime.h>
#include <stingraykit/string/ToString.h>


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
		std::string					_messageHolder;
		const char*					_message;
		optional<ToolkitWhere>		_where;
		u64							_thresholdMs;
		u64							_criticalMs;

		ElapsedTime					_et;

	public:
		explicit Profiler(const std::string& message, u64 thresholdMs = 0, u64 criticalMs = std::numeric_limits<size_t>::max()) :
			_messageHolder(message), _thresholdMs(thresholdMs), _criticalMs(criticalMs)
		{ _message = _messageHolder.c_str(); }

		explicit Profiler(const Detail::NamedLoggerAccessor& logger, const char* message, u64 thresholdMs = 0, u64 criticalMs = std::numeric_limits<size_t>::max()) :
			_logger(logger), _message(message), _thresholdMs(thresholdMs), _criticalMs(criticalMs)
		{ }

		explicit Profiler(const Detail::NamedLoggerAccessor& logger, const char* message, ToolkitWhere where, u64 thresholdMs = 0, u64 criticalMs = std::numeric_limits<size_t>::max()) :
			_logger(logger), _message(message), _where(where), _thresholdMs(thresholdMs), _criticalMs(criticalMs)
		{ }

		explicit Profiler(const Detail::NamedLoggerAccessor& logger, const std::string& message, u64 thresholdMs = 0, u64 criticalMs = std::numeric_limits<size_t>::max()) :
			_logger(logger), _messageHolder(message), _thresholdMs(thresholdMs), _criticalMs(criticalMs)
		{ _message = _messageHolder.c_str(); }

		explicit Profiler(const Detail::NamedLoggerAccessor& logger, const std::string& message, ToolkitWhere where, u64 thresholdMs = 0, u64 criticalMs = std::numeric_limits<size_t>::max()) :
			_logger(logger), _messageHolder(message), _where(where), _thresholdMs(thresholdMs), _criticalMs(criticalMs)
		{ _message = _messageHolder.c_str(); }

		~Profiler()
		{
			try
			{
				u64 ms = _et.ElapsedMilliseconds();
				if (ms > _thresholdMs)
				{
					LogLevel ll = (ms > _criticalMs) ? LogLevel::Warning : LogLevel::Info;
					if (_where)
						_logger.Stream(ll) << ms << " milliseconds - " << *_where << ": " <<_message;
					else
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
	Profiler STINGRAYKIT_CAT(detail_profiler, __LINE__)(STINGRAYKIT_STATIC_LOGGER, (Message_), STINGRAYKIT_WHERE, Milliseconds_);

#define STINGRAYKIT_PROFILER_NO_WHERE(Milliseconds_, Message_) \
	DETAIL_DECLARE_STATIC_LOGGER_ACCESSOR; \
	Profiler STINGRAYKIT_CAT(detail_profiler, __LINE__)(STINGRAYKIT_STATIC_LOGGER, (Message_), Milliseconds_);

#define STINGRAYKIT_PROFILER_CRITICAL_TIME(Milliseconds_, Message_) \
	DETAIL_DECLARE_STATIC_LOGGER_ACCESSOR; \
	Profiler STINGRAYKIT_CAT(detail_profiler, __LINE__)(STINGRAYKIT_STATIC_LOGGER, (Message_), STINGRAYKIT_WHERE, Milliseconds_, Milliseconds_);

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
