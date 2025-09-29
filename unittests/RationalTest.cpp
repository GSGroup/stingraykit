// Copyright (c) 2011 - 2025, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/Rational.h>

#include <gtest/gtest.h>

using namespace stingray;

#define CHECK_RATIONAL_NUM_DENUM_VALUES(Rational, NumValue, DenumValue) \
		do { \
			ASSERT_EQ(Rational.Num(), NumValue); \
			ASSERT_EQ(Rational.Denum(), DenumValue); \
		} while (false)

TEST(RationalTest, Ctor)
{
	{
		Rational value;
		CHECK_RATIONAL_NUM_DENUM_VALUES(value, 0, 1);
	}

	{
		Rational value(313);
		CHECK_RATIONAL_NUM_DENUM_VALUES(value, 313, 1);
	}

	{
		Rational value(5, 17);
		CHECK_RATIONAL_NUM_DENUM_VALUES(value, 5, 17);
	}

	{
		Rational value(30, 153);
		CHECK_RATIONAL_NUM_DENUM_VALUES(value, 10, 51);
	}

	{
		Rational value(-30, 153);
		CHECK_RATIONAL_NUM_DENUM_VALUES(value, -10, 51);
	}

	{
		Rational value(30, -153);
		CHECK_RATIONAL_NUM_DENUM_VALUES(value, -10, 51);
	}

	{
		Rational value(-30, -153);
		CHECK_RATIONAL_NUM_DENUM_VALUES(value, 10, 51);
	}

	ASSERT_THROW(Rational(313, 0), ArgumentException);
}
