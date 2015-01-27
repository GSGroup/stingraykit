#ifndef STINGRAYKIT_LOG_LOGGERMESSAGE_H
#define STINGRAYKIT_LOG_LOGGERMESSAGE_H


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

	public:
		LoggerMessage(const LogLevel& logLevel, const std::string& message);
		LoggerMessage(const std::string& loggerName, const LogLevel& logLevel, const std::string& message);

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
