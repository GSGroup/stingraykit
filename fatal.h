#ifndef STINGRAYKIT_FATAL_H
#define STINGRAYKIT_FATAL_H

#include <stingray/toolkit/toolkit.h>

#define STINGRAYKIT_FATAL(message) \
	do { \
		std::string msg = STINGRAYKIT_WHERE.ToString() + std::string(": ") + std::string(message); \
		stingray::DebuggingHelper::TerminateWithMessage(msg); \
	} while(0)

#endif
