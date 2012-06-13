#ifndef STINGRAY_TOOLKIT_FATAL_H__
#define STINGRAY_TOOLKIT_FATAL_H__

#include <stingray/toolkit/toolkit.h>

#define TOOLKIT_FATAL(message) \
	do { \
		std::string msg = TOOLKIT_WHERE + std::string(": ") + std::string(message); \
		DebuggingHelper::TerminateWithMessage(msg); \
	} while(0)

#endif
