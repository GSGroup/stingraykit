#ifndef STINGRAY_TOOLKIT_LOG_SYSTEMLOGGER_H
#define STINGRAY_TOOLKIT_LOG_SYSTEMLOGGER_H


#include <stingray/toolkit/log/LogLevel.h>

namespace stingray
{

	struct SystemLogger
	{
		static void Log(LogLevel logLevel, const std::string& message) throw ();
		static void Log(const std::string& loggerName, LogLevel logLevel, const std::string& message) throw ();
	};

}

#endif
