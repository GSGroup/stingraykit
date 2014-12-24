#ifndef STINGRAY_TOOLKIT_LOG_LOGGERMESSAGE_H
#define STINGRAY_TOOLKIT_LOG_LOGGERMESSAGE_H


#include <stingray/toolkit/log/LogLevel.h>
#include <stingray/toolkit/time/Time.h>
#include <stingray/toolkit/optional.h>
#include <stingray/toolkit/compare/MemberListComparer.h>

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

		bool HasLoggerName() const { return _loggerName; }
		std::string GetLoggerName() const;

		LogLevel GetLogLevel() const { return _logLevel; }
		Time GetTime() const { return _time; }
		std::string GetThreadName() const { return _threadName; }
		std::string GetMessage() const { return _message; }

		std::string ToString() const;

		bool operator < (const LoggerMessage &other) const
		{ return CompareMembersLess(&LoggerMessage::_loggerName, &LoggerMessage::_logLevel, &LoggerMessage::_time, &LoggerMessage::_threadName, &LoggerMessage::_message)(*this, other); }

		TOOLKIT_GENERATE_COMPARISON_OPERATORS_FROM_LESS(LoggerMessage);
	};

	/** @} */

}


#endif
