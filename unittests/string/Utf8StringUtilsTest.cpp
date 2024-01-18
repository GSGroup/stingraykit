// Copyright (c) 2011 - 2024, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/string/Utf8StringUtils.h>

#include <gtest/gtest.h>

using namespace stingray;

TEST(Utf8StringUtilsTest, Utf8Strlen)
{
	ASSERT_EQ(Utf8Strlen("TestString"), 10U);
	ASSERT_EQ(Utf8Strlen("TestСтрока"), 10U);
	ASSERT_EQ(Utf8Strlen("ТестString"), 10U);
}


TEST(Utf8StringUtilsTest, Utf8Validate)
{
	ASSERT_TRUE(Utf8Validate("TestString"));
	ASSERT_TRUE(Utf8Validate("TestСтрока"));
	ASSERT_TRUE(Utf8Validate("ТестString"));

	ASSERT_FALSE(Utf8Validate("Test\x91\xe2\xe0\xae\xaa\xa0")); // TestСтрока in CP866
	ASSERT_FALSE(Utf8Validate("\x92\xa5\xe1\xe2String")); // ТестString in CP866
}


TEST(Utf8StringUtilsTest, Utf8SubstringByChars)
{
	ASSERT_EQ(Utf8Substring<false>("TestString", 0, std::string::npos), "TestString");
	ASSERT_EQ(Utf8Substring<false>("TestСтрока", 0, std::string::npos), "TestСтрока");
	ASSERT_EQ(Utf8Substring<false>("ТестString", 0, std::string::npos), "ТестString");

	ASSERT_EQ(Utf8Substring<false>("TestString", 10, std::string::npos), "");
	ASSERT_EQ(Utf8Substring<false>("TestСтрока", 16, std::string::npos), "");
	ASSERT_EQ(Utf8Substring<false>("ТестString", 14, std::string::npos), "");

	ASSERT_ANY_THROW(Utf8Substring<false>("TestString", 11, std::string::npos));
	ASSERT_ANY_THROW(Utf8Substring<false>("TestСтрока", 17, std::string::npos));
	ASSERT_ANY_THROW(Utf8Substring<false>("ТестString", 15, std::string::npos));

	ASSERT_EQ(Utf8Substring<false>("TestString", 0, 6), "TestSt");
	ASSERT_EQ(Utf8Substring<false>("TestСтрока", 0, 6), "TestС");
	ASSERT_EQ(Utf8Substring<false>("ТестString", 0, 6), "Тес");

	ASSERT_EQ(Utf8Substring<false>("TestString", 2, 4), "stSt");
	ASSERT_EQ(Utf8Substring<false>("TestСтрока", 2, 4), "stС");
	ASSERT_EQ(Utf8Substring<false>("ТестString", 2, 4), "ес");

	ASSERT_EQ(Utf8Substring<false>("TestString", 0, 7), "TestStr");
	ASSERT_EQ(Utf8Substring<false>("TestСтрока", 0, 7), "TestС");
	ASSERT_EQ(Utf8Substring<false>("ТестString", 0, 7), "Тес");

	ASSERT_EQ(Utf8Substring<false>("TestString", 3, 5), "tStri");
	ASSERT_EQ(Utf8Substring<false>("TestСтрока", 3, 5), "tСт");
	ASSERT_EQ(Utf8Substring<false>("ТестString", 3, 5), "стS");
}


TEST(Utf8StringUtilsTest, Utf8SubstringBySymbols)
{
	ASSERT_EQ(Utf8Substring<true>("TestString", 0, std::string::npos), "TestString");
	ASSERT_EQ(Utf8Substring<true>("TestСтрока", 0, std::string::npos), "TestСтрока");
	ASSERT_EQ(Utf8Substring<true>("ТестString", 0, std::string::npos), "ТестString");

	ASSERT_EQ(Utf8Substring<true>("TestString", 10, std::string::npos), "");
	ASSERT_EQ(Utf8Substring<true>("TestСтрока", 10, std::string::npos), "");
	ASSERT_EQ(Utf8Substring<true>("ТестString", 10, std::string::npos), "");

	ASSERT_ANY_THROW(Utf8Substring<true>("TestString", 11, std::string::npos));
	ASSERT_ANY_THROW(Utf8Substring<true>("TestСтрока", 11, std::string::npos));
	ASSERT_ANY_THROW(Utf8Substring<true>("ТестString", 11, std::string::npos));

	ASSERT_EQ(Utf8Substring<true>("TestString", 0, 6), "TestSt");
	ASSERT_EQ(Utf8Substring<true>("TestСтрока", 0, 6), "TestСт");
	ASSERT_EQ(Utf8Substring<true>("ТестString", 0, 6), "ТестSt");

	ASSERT_EQ(Utf8Substring<true>("TestString", 2, 4), "stSt");
	ASSERT_EQ(Utf8Substring<true>("TestСтрока", 2, 4), "stСт");
	ASSERT_EQ(Utf8Substring<true>("ТестString", 2, 4), "стSt");

	ASSERT_EQ(Utf8Substring<true>("TestString", 0, 7), "TestStr");
	ASSERT_EQ(Utf8Substring<true>("TestСтрока", 0, 7), "TestСтр");
	ASSERT_EQ(Utf8Substring<true>("ТестString", 0, 7), "ТестStr");

	ASSERT_EQ(Utf8Substring<true>("TestString", 3, 5), "tStri");
	ASSERT_EQ(Utf8Substring<true>("TestСтрока", 3, 5), "tСтро");
	ASSERT_EQ(Utf8Substring<true>("ТестString", 3, 5), "тStri");
}
