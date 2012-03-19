#ifndef STINGRAY_TOOLKIT_FATAL_H__
#define STINGRAY_TOOLKIT_FATAL_H__

#include <stingray/log/Logger.h>
#include <stingray/toolkit/toolkit.h>

#define TOOLKIT_FATAL(message) \
	do { \
		Logger::Error() << "Fatal error at " << TOOLKIT_WHERE << ": " << (message); \
		std::terminate(); \
	} while(0)

#endif
