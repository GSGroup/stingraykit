#include <stingraykit/log/LoggerMessage.h>

#include <stingraykit/compare/MemberListComparer.h>
#include <stingraykit/string/StringFormat.h>
#include <stingraykit/thread/Thread.h>


namespace stingray
{

	LoggerMessage::LoggerMessage(const LogLevel& logLevel, const std::string& message)
		: _logLevel(logLevel), _time(Time::Now()), _threadName(Thread::GetCurrentThreadName()), _message(message)
	{ }


	LoggerMessage::LoggerMessage(const std::string& loggerName, const LogLevel& logLevel, const std::string& message)
		: _loggerName(loggerName), _logLevel(logLevel), _time(Time::Now()), _threadName(Thread::GetCurrentThreadName()), _message(message)
	{ }


	std::string LoggerMessage::GetLoggerName() const
	{
		STINGRAYKIT_CHECK(HasLoggerName(), LogicException());
		return *_loggerName;
	}


	std::string LoggerMessage::ToString() const
	{
		std::string threadName = _threadName.empty() ? "__undefined__" : _threadName;
		if (_loggerName)
			return StringFormat("[%1%] [%2%] {%3%} [%4%] %5%", _time.ToString(), _logLevel, threadName, *_loggerName, _message);

		return StringFormat("[%1%] [%2%] {%3%} %4%", _time.ToString(), _logLevel, threadName, _message);
	}


	bool LoggerMessage::operator < (const LoggerMessage &other) const
	{ return CompareMembersLess(&LoggerMessage::_loggerName, &LoggerMessage::_logLevel, &LoggerMessage::_time, &LoggerMessage::_threadName, &LoggerMessage::_message)(*this, other); }

}
