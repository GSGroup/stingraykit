// Copyright (c) 2011 - 2024, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/string/ToString.h>
#include <stingraykit/BigNum.h>

#include <gtest/gtest.h>

using namespace stingray;

TEST(BigNumTest, OutOfBoundsException)
{
	BigUnsignedInteger x = 1000;
	BigUnsignedInteger y = x * 1000;
	ASSERT_TRUE(y == 1000000);

	BigUnsignedInteger a = 65536;
	BigUnsignedInteger result = a * a * a * a * a * a * a * a;
	ASSERT_TRUE(result == BigUnsignedInteger("340282366920938463463374607431768211456", 10));
	ASSERT_TRUE(result / "123456789" == "2756287197141815048043851257396");
	ASSERT_TRUE((result % "123456789").ToPrimitive<int>() == 45550012);
	ASSERT_TRUE(result + (result / "123456789") == "340282369677225660605189655475619468852");
	ASSERT_TRUE((result - "340282366920938463463374607431768210432").ToPrimitive<u16>() == 1024);

	BigUnsignedInteger pattern1 = "0x5c5c5c5c5c5c5c5c5c5c5c5c";
	BigUnsignedInteger pattern2 = "0x363636363636363636363636";
	ASSERT_TRUE((pattern1 & pattern2) == "0x141414141414141414141414");
	ASSERT_TRUE((pattern1 | pattern2) == "0x7e7e7e7e7e7e7e7e7e7e7e7e");
	ASSERT_TRUE((pattern1 ^ pattern2) == "0x6a6a6a6a6a6a6a6a6a6a6a6a");
	ASSERT_TRUE(pattern1 >> 17 == "0x2e2e2e2e2e2e2e2e2e2e");
	ASSERT_TRUE(pattern1 << 10 == "0x171717171717171717171717000");

	BigUnsignedInteger v("abcdefabcdefabcdefabcdefabcdef0123456789", 16);
	ASSERT_TRUE((v * (1 << 16)) == "0xabcdefabcdefabcdefabcdefabcdef01234567890000");
}
