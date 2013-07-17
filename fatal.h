#ifndef STINGRAY_TOOLKIT_FATAL_H
#define STINGRAY_TOOLKIT_FATAL_H

#include <stingray/toolkit/toolkit.h>

#define TOOLKIT_FATAL(message) \
	do { \
		std::string msg = TOOLKIT_WHERE.ToString() + std::string(": ") + std::string(message); \
		DebuggingHelper::TerminateWithMessage(msg); \
	} while(0)

#endif
