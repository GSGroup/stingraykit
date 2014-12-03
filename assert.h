#ifndef STINGRAY_TOOLKIT_ASSERT_H
#define STINGRAY_TOOLKIT_ASSERT_H

#include <stingray/toolkit/fatal.h>

#define TOOLKIT_ASSERT(value) \
		do { if (!(value)) TOOLKIT_FATAL("assertion " #value " failed"); } while (false)

#endif
