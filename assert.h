#ifndef STINGRAYKIT_ASSERT_H
#define STINGRAYKIT_ASSERT_H

#include <stingray/toolkit/fatal.h>

#define STINGRAYKIT_ASSERT(value) \
		do { if (STINGRAYKIT_UNLIKELY(!(value))) STINGRAYKIT_FATAL("assertion " #value " failed"); } while (false)

#define STINGRAYKIT_DEBUG_ASSERT(value) \
		do { STINGRAYKIT_DEBUG_ONLY(if (STINGRAYKIT_UNLIKELY(!(value))) STINGRAYKIT_FATAL("assertion " #value " failed")); } while (false)

#endif
