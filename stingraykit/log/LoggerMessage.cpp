// Copyright (c) 2011 - 2017, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/log/LoggerMessage.h>

#include <stingraykit/compare/MemberListComparer.h>
#include <stingraykit/string/StringFormat.h>
#include <stingraykit/thread/Thread.h>


namespace stingray
{

	LoggerMessage::LoggerMessage(const LogLevel& logLevel, const std::string& message, bool highlight) :
		_logLevel(logLevel), _time(Time::Now()),
		_threadName(Thread::GetCurrentThreadName().empty() ? "__undefined__" : Thread::GetCurrentThreadName()),
		_message(message), _highlight(highlight)
	{ }


	LoggerMessage::LoggerMessage(const std::string& loggerName, const LogLevel& logLevel, const std::string& message, bool highlight) :
		_loggerName(loggerName), _logLevel(logLevel), _time(Time::Now()),
		_threadName(Thread::GetCurrentThreadName().empty() ? "__undefined__" : Thread::GetCurrentThreadName()),
		_message(message), _highlight(highlight)
	{ }


	std::string LoggerMessage::GetLoggerName() const
	{
		STINGRAYKIT_CHECK(HasLoggerName(), LogicException());
		return *_loggerName;
	}


	std::string LoggerMessage::ToString() const
	{
		if (_loggerName)
			return StringFormat("[%1%] [%2%] {%3%} [%4%] %5%", _time.ToString(), _logLevel, _threadName, *_loggerName, _message);

		return StringFormat("[%1%] [%2%] {%3%} %4%", _time.ToString(), _logLevel, _threadName, _message);
	}


	bool LoggerMessage::operator < (const LoggerMessage &other) const
	{ return CompareMembersLess(&LoggerMessage::_loggerName, &LoggerMessage::_logLevel, &LoggerMessage::_time, &LoggerMessage::_threadName, &LoggerMessage::_message)(*this, other); }

}
