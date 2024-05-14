#ifndef STINGRAYKIT_MATH_H
#define STINGRAYKIT_MATH_H

// Copyright (c) 2011 - 2024, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/exception.h>

namespace stingray
{

	constexpr s8 Abs(s8 val) { return val >= 0 ? val : -val; }
	constexpr s16 Abs(s16 val) { return val >= 0 ? val : -val; }
	constexpr s32 Abs(s32 val) { return val >= 0 ? val : -val; }
	constexpr s64 Abs(s64 val) { return val >= 0 ? val : -val; }

	template < typename T > constexpr T AlignUp(T value, T boundary)
	{
		STINGRAYKIT_CHECK(boundary != 0, ArgumentException("boundary"));
		return boundary * ((value + boundary - 1) / boundary);
	}

	template < typename T > constexpr T AlignDown(T value, T boundary)
	{
		STINGRAYKIT_CHECK(boundary != 0, ArgumentException("boundary"));
		return boundary * (value / boundary);
	}

	constexpr size_t Gcd(size_t a, size_t b) { return b ? Gcd(b, a % b) : a; }

	constexpr size_t Lcm(size_t a, size_t b) { return a / Gcd(a, b) * b; }

	template < typename T > constexpr T Max(T a, T b) { return std::max<T>(a, b); }

	template < typename T > constexpr T Min(T a, T b) { return std::min<T>(a, b); }

	struct FractionInfo
	{
		u64				Fraction = 0;
		unsigned		Precision = 0;
		bool			IsOverflow = false;
	};

	FractionInfo CalculateFractionRemainder(u64 dividend, u64 divisor, unsigned targetPrecision, unsigned radix = 10);

	FractionInfo ParseDecimalFraction(const std::string& fractionStr, unsigned targetPrecision);

}

#endif
