#include <stingray/toolkit/log/LoggerMessage.h>

#include <stingray/toolkit/thread/Thread.h>
#include <stingray/toolkit/StringFormat.h>


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
		TOOLKIT_CHECK(HasLoggerName(), LogicException());
		return *_loggerName;
	}


	std::string LoggerMessage::ToString() const
	{
		if (_loggerName)
			return StringFormat("[%1%] [%2%] {%3%} [%4%] %5%", _time.ToString(), _logLevel, _threadName, *_loggerName, _message);

		return StringFormat("[%1%] [%2%] {%3%} %4%", _time.ToString(), _logLevel, _threadName, _message);
	}

}
