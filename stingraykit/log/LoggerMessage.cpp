// Copyright (c) 2011 - 2025, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/log/LoggerMessage.h>

#include <stingraykit/compare/MemberListComparer.h>
#include <stingraykit/string/ToString.h>
#include <stingraykit/thread/Thread.h>

namespace stingray
{

	LoggerMessage::LoggerMessage(const LogLevel& logLevel, const std::string& message, bool highlight)
		:	_logLevel(logLevel),
			_time(Time::Now()),
			_threadName(Thread::GetCurrentThreadName()),
			_message(message),
			_highlight(highlight)
	{
		if (_threadName.empty())
			_threadName = "__undefined__";
	}


	LoggerMessage::LoggerMessage(const std::string& loggerName, const LogLevel& logLevel, const std::string& message, bool highlight)
		:	_loggerName(loggerName),
			_logLevel(logLevel),
			_time(Time::Now()),
			_threadName(Thread::GetCurrentThreadName()),
			_message(message),
			_highlight(highlight)
	{
		if (_threadName.empty())
			_threadName = "__undefined__";
	}


	std::string LoggerMessage::ToString() const
	{
		StringBuilder sb;
		sb % "[" % _time % "] [" % _logLevel % "] {" % _threadName % "} ";

		if (_loggerName)
			sb % "[" % _loggerName % "] ";

		sb % _message;
		return sb;
	}


	bool LoggerMessage::operator < (const LoggerMessage& other) const
	{ return CompareMembersLess(&LoggerMessage::_loggerName, &LoggerMessage::_logLevel, &LoggerMessage::_time, &LoggerMessage::_threadName, &LoggerMessage::_message)(*this, other); }

}
