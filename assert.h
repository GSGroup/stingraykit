#ifndef STINGRAY_ASSERT_H__
#define STINGRAY_ASSERT_H__

#include <dvrlib/toolkit/fatal.h>

#define TOOLKIT_ASSERT(value) if (!(value)) TOOLKIT_FATAL("assertion " #value " failed")

#endif
