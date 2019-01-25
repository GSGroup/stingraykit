#ifndef STINGRAYKIT_DIAGNOSTICS_PROFILER_H
#define STINGRAYKIT_DIAGNOSTICS_PROFILER_H

// Copyright (c) 2011 - 2019, GS Group, https://github.com/GSGroup
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
		class MessageHolder
		{
		private:
			std::string _messageHolder;
			const char* _message;

		public:
			MessageHolder(const char* message) : _message(message)
			{ }

			MessageHolder(const std::string& message) : _messageHolder(message)
			{ _message = _messageHolder.c_str(); }

			const char* Get() const
			{ return _message; }
		};

	private:
		Detail::NamedLoggerAccessor	_logger;
		LogLevel					_logLevel;
		MessageHolder				_message;
		optional<ToolkitWhere>		_where;
		u64							_thresholdMs;

		ElapsedTime					_et;

	public:
		Profiler(const std::string& message, u64 thresholdMs) :
			_logLevel(LogLevel::Info), _message(message), _thresholdMs(thresholdMs)
		{ }

		Profiler(const Detail::NamedLoggerAccessor& logger, const char* message, u64 thresholdMs) :
			_logger(logger), _logLevel(LogLevel::Info), _message(message), _thresholdMs(thresholdMs)
		{ }

		Profiler(const Detail::NamedLoggerAccessor& logger, const std::string& message, u64 thresholdMs) :
			_logger(logger), _logLevel(LogLevel::Info), _message(message), _thresholdMs(thresholdMs)
		{ }

		Profiler(const Detail::NamedLoggerAccessor& logger, LogLevel logLevel, const char* message, u64 thresholdMs) :
			_logger(logger), _logLevel(logLevel), _message(message), _thresholdMs(thresholdMs)
		{ }

		Profiler(const Detail::NamedLoggerAccessor& logger, LogLevel logLevel, const std::string& message, u64 thresholdMs) :
			_logger(logger), _logLevel(logLevel), _message(message), _thresholdMs(thresholdMs)
		{ }

		Profiler(const Detail::NamedLoggerAccessor& logger, const char* message, ToolkitWhere where, u64 thresholdMs) :
			_logger(logger), _logLevel(LogLevel::Info), _message(message), _where(where), _thresholdMs(thresholdMs)
		{ }

		Profiler(const Detail::NamedLoggerAccessor& logger, const std::string& message, ToolkitWhere where, u64 thresholdMs) :
			_logger(logger), _logLevel(LogLevel::Info), _message(message), _where(where), _thresholdMs(thresholdMs)
		{ }

		Profiler(const Detail::NamedLoggerAccessor& logger, LogLevel logLevel, const char* message, ToolkitWhere where, u64 thresholdMs) :
			_logger(logger), _logLevel(logLevel), _message(message), _where(where), _thresholdMs(thresholdMs)
		{ }

		Profiler(const Detail::NamedLoggerAccessor& logger, LogLevel logLevel, const std::string& message, ToolkitWhere where, u64 thresholdMs) :
			_logger(logger), _logLevel(logLevel), _message(message), _where(where), _thresholdMs(thresholdMs)
		{ }

		~Profiler()
		{
			try
			{
				u64 ms = _et.ElapsedMilliseconds();
				if (ms > _thresholdMs)
				{
					if (_where)
						_logger.Stream(_logLevel) << ms << " milliseconds - " << *_where << ": " << _message.Get();
					else
						_logger.Stream(_logLevel) << ms << " milliseconds - " << _message.Get();
				}
			}
			catch (const std::exception& ex)
			{ Logger::Error() << "Exception in " << STINGRAYKIT_WHERE << ": " << ex; }
		}
	};


#define STINGRAYKIT_PROFILER(Milliseconds_, Message_) \
	DETAIL_DECLARE_STATIC_LOGGER_ACCESSOR; \
	stingray::Profiler STINGRAYKIT_CAT(detail_profiler, __LINE__)(STINGRAYKIT_STATIC_LOGGER, (Message_), STINGRAYKIT_WHERE, Milliseconds_);

#define STINGRAYKIT_PROFILER_DEBUG(Milliseconds_, Message_) \
	DETAIL_DECLARE_STATIC_LOGGER_ACCESSOR; \
	stingray::Profiler STINGRAYKIT_CAT(detail_profiler, __LINE__)(STINGRAYKIT_STATIC_LOGGER, LogLevel::Debug, (Message_), STINGRAYKIT_WHERE, Milliseconds_);

#define STINGRAYKIT_PROFILER_NO_WHERE(Milliseconds_, Message_) \
	DETAIL_DECLARE_STATIC_LOGGER_ACCESSOR; \
	stingray::Profiler STINGRAYKIT_CAT(detail_profiler, __LINE__)(STINGRAYKIT_STATIC_LOGGER, (Message_), Milliseconds_);

#define STINGRAYKIT_PROFILER_DEBUG_NO_WHERE(Milliseconds_, Message_) \
	DETAIL_DECLARE_STATIC_LOGGER_ACCESSOR; \
	stingray::Profiler STINGRAYKIT_CAT(detail_profiler, __LINE__)(STINGRAYKIT_STATIC_LOGGER, LogLevel::Debug, (Message_), Milliseconds_);

#define STINGRAYKIT_PROFILE(Milliseconds_, Call_) \
	do { \
		STINGRAYKIT_PROFILER(Milliseconds_, #Call_); \
		Call_; \
	} while (false)

#define STINGRAYKIT_PROFILE_CRITICAL_TIME(Milliseconds_, Call_) \
	do { \
		STINGRAYKIT_PROFILER(Milliseconds_, #Call_); \
		Call_; \
	} while (false)

	/** @} */

}


#endif
