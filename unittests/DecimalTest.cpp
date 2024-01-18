// Copyright (c) 2011 - 2024, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/Decimal.h>

#include <gtest/gtest.h>

using namespace stingray;


TEST(DecimalTest, Addition)
{
	EXPECT_EQ(Decimal(33156, 3) + Decimal(23457, 3), Decimal(56613, 3));
	EXPECT_EQ(Decimal(11, 2) + Decimal(23, 1), Decimal(241, 2));
	EXPECT_EQ(Decimal(3451, 1) + Decimal(23, 2), Decimal(34533, 2));

	EXPECT_EQ(Decimal(-123, 2) + Decimal(476, 1), Decimal(4637, 2));
	EXPECT_EQ(Decimal(12, 0) + Decimal(-23212, 4), Decimal(96788, 4));
	EXPECT_EQ(Decimal(-8721, 2) + Decimal(-192322, 2), Decimal(-201043, 2));
}


TEST(DecimalTest, Subtraction)
{
	EXPECT_EQ(Decimal(33156, 3) - Decimal(23457, 3), Decimal(9699, 3));
	EXPECT_EQ(Decimal(11, 2) - Decimal(23, 1), Decimal(-219, 2));
	EXPECT_EQ(Decimal(3451, 1) - Decimal(23, 2), Decimal(34487, 2));

	EXPECT_EQ(Decimal(-123, 2) - Decimal(476, 1), Decimal(-4883, 2));
	EXPECT_EQ(Decimal(12, 0) - Decimal(-23212, 4), Decimal(143212, 4));
	EXPECT_EQ(Decimal(-8721, 2) - Decimal(-192322, 2), Decimal(183601, 2));
}


TEST(DecimalTest, Multiplication)
{
	EXPECT_EQ(Decimal(33156, 3) * Decimal(23457, 3), Decimal(777740292, 6));
	EXPECT_EQ(Decimal(11, 2) * Decimal(23, 1), Decimal(253, 3));
	EXPECT_EQ(Decimal(3451, 1) * Decimal(23, 2), Decimal(79373, 3));

	EXPECT_EQ(Decimal(-123, 2) * Decimal(476, 1), Decimal(-58548, 3));
	EXPECT_EQ(Decimal(12, 0) * Decimal(-23212, 4), Decimal(-278544, 4));
	EXPECT_EQ(Decimal(-8721, 2) * Decimal(-192322, 2), Decimal(1677240162, 4));
}


TEST(DecimalTest, FromString)
{
	EXPECT_EQ(Decimal::FromString("0"), Decimal(0, 0));
	EXPECT_EQ(Decimal::FromString("-0"), Decimal(0, 0));
	EXPECT_EQ(Decimal::FromString("50"), Decimal(50, 0));
	EXPECT_EQ(Decimal::FromString("-50"), Decimal(-50, 0));

	EXPECT_EQ(Decimal::FromString("0.1"), Decimal(1, 1));
	EXPECT_EQ(Decimal::FromString("-0.1"), Decimal(-1, 1));
	EXPECT_EQ(Decimal::FromString("0.0123"), Decimal(123, 4));
	EXPECT_EQ(Decimal::FromString("-0.0123"), Decimal(-123, 4));

	EXPECT_EQ(Decimal::FromString("5.123000"), Decimal(5123, 3));
	EXPECT_EQ(Decimal::FromString("-3311.012300"), Decimal(-33110123, 4));
}


TEST(DecimalTest, ToString)
{
	EXPECT_EQ(Decimal(0, 0).ToString(), "0");
	EXPECT_EQ(Decimal(18223, 0).ToString(), "18223");
	EXPECT_EQ(Decimal(-18223, 0).ToString(), "-18223");
	EXPECT_EQ(Decimal(33, 1).ToString(), "3.3");
	EXPECT_EQ(Decimal(-4587, 3).ToString(), "-4.587");
	EXPECT_EQ(Decimal(100, 3).ToString(), "0.1");
	EXPECT_EQ(Decimal(-51000, 4).ToString(), "-5.1");
}