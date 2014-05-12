#ifndef STINGRAY_TOOLKIT_MATH_H
#define STINGRAY_TOOLKIT_MATH_H


#include <stingray/toolkit/exception.h>

namespace stingray
{

	inline s8 abs(s8 val)	{ return val >= 0 ? val : -val; }
	inline s16 abs(s16 val)	{ return val >= 0 ? val : -val; }
	inline s32 abs(s32 val)	{ return val >= 0 ? val : -val; }
	inline s64 abs(s64 val)	{ return val >= 0 ? val : -val; }

	inline size_t AlignUp(size_t value, size_t boundary)
	{
		TOOLKIT_CHECK(boundary != 0, ArgumentException("boundary"));
		return boundary * ((value + boundary - 1) / boundary);
	}

	inline size_t AlignDown(size_t value, size_t boundary)
	{
		TOOLKIT_CHECK(boundary != 0, ArgumentException("boundary"));
		return boundary * (value / boundary);
	}

}

#endif
