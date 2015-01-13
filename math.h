#ifndef STINGRAYKIT_MATH_H
#define STINGRAYKIT_MATH_H


#include <stingray/toolkit/exception.h>

namespace stingray
{

	inline s8 abs(s8 val)	{ return val >= 0 ? val : -val; }
	inline s16 abs(s16 val)	{ return val >= 0 ? val : -val; }
	inline s32 abs(s32 val)	{ return val >= 0 ? val : -val; }
	inline s64 abs(s64 val)	{ return val >= 0 ? val : -val; }

	inline size_t AlignUp(size_t value, size_t boundary)
	{
		STINGRAYKIT_CHECK(boundary != 0, ArgumentException("boundary"));
		return boundary * ((value + boundary - 1) / boundary);
	}

	inline size_t AlignDown(size_t value, size_t boundary)
	{
		STINGRAYKIT_CHECK(boundary != 0, ArgumentException("boundary"));
		return boundary * (value / boundary);
	}

	inline size_t Gcd(size_t a, size_t b) { return b ? Gcd(b, a % b) : a; }

	inline size_t Lcm(size_t a, size_t b) { return a / Gcd(a, b) * b; }

}

#endif
