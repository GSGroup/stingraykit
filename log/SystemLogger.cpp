#include <stingray/toolkit/log/SystemLogger.h>

#include <cstdio>
#include <string>


namespace stingray
{

	void SystemLogger::Log(LogLevel logLevel, const std::string& message) throw ()
	{
#ifndef PRODUCTION_BUILD
		std::fprintf(stderr, "SystemLogger: [%s] %s\n", logLevel.ToString().c_str(), message.c_str());
#endif
	}

	void SystemLogger::Log(const std::string& loggerName, LogLevel logLevel, const std::string& message) throw ()
	{
#ifndef PRODUCTION_BUILD
		std::fprintf(stderr, "SystemLogger: [%s] [%s] %s\n", logLevel.ToString().c_str(), loggerName.c_str(), message.c_str());
#endif
	}

}
