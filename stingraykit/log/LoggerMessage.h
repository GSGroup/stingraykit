#ifndef STINGRAYKIT_LOG_LOGGERMESSAGE_H
#define STINGRAYKIT_LOG_LOGGERMESSAGE_H

// Copyright (c) 2011 - 2018, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.


#include <stingraykit/log/LogLevel.h>
#include <stingraykit/time/Time.h>
#include <stingraykit/optional.h>

#include <functional>


namespace stingray
{

	/**
	 * @addtogroup toolkit_log
	 * @{
	 */

	class LoggerMessage
	{
	private:
		optional<std::string>	_loggerName;
		LogLevel				_logLevel;
		Time					_time;
		std::string				_threadName;
		std::string				_message;
		bool					_highlight;

	public:
		LoggerMessage(const LogLevel& logLevel, const std::string& message, bool highlight);
		LoggerMessage(const std::string& loggerName, const LogLevel& logLevel, const std::string& message, bool highlight);

		bool Highlight() const				{ return _highlight; }
		bool HasLoggerName() const			{ return _loggerName; }
		std::string GetLoggerName() const;

		LogLevel GetLogLevel() const		{ return _logLevel; }
		Time GetTime() const				{ return _time; }
		std::string GetThreadName() const	{ return _threadName; }
		std::string GetMessage() const		{ return _message; }

		std::string ToString() const;

		bool operator < (const LoggerMessage &other) const;
		STINGRAYKIT_GENERATE_COMPARISON_OPERATORS_FROM_LESS(LoggerMessage);
	};

	/** @} */

}


#endif
