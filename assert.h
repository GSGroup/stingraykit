#ifndef STINGRAY_TOOLKIT_ASSERT_H
#define STINGRAY_TOOLKIT_ASSERT_H

#include <stingray/toolkit/fatal.h>

#define TOOLKIT_ASSERT(value) \
		do { if (STINGRAY_UNLIKELY(!(value))) TOOLKIT_FATAL("assertion " #value " failed"); } while (false)

#define TOOLKIT_DEBUG_ASSERT(value) \
		do { TOOLKIT_DEBUG_ONLY(if (STINGRAY_UNLIKELY(!(value))) TOOLKIT_FATAL("assertion " #value " failed")); } while (false)

#endif
