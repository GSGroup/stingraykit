// Copyright (c) 2011 - 2025, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/function/bind.h>
#include <stingraykit/string/AhoCorasick.h>
#include <stingraykit/string/Hex.h>
#include <stingraykit/string/StringUtils.h>

#include <unittests/RangeMatcher.h>

#include <gmock/gmock-more-matchers.h>

using namespace stingray;

using ::testing::ElementsAre;
using ::testing::IsEmpty;

TEST(StringUtilsTest, Contains_Substring_ReturnsTrue)
{
	ASSERT_TRUE(Contains("Lorem Ipsum", "Lorem"));
	ASSERT_TRUE(Contains("Lorem Ipsum", "Ipsum"));
}


TEST(StringUtilsTest, Contains_NotSubstring_ReturnFalse)
{
	ASSERT_FALSE(Contains("Lorem Ipsum", "dolor"));
	ASSERT_FALSE(Contains("Lorem Ipsum", "sit amet"));
}


TEST(StringUtilsTest, Contains_SameString_ReturnsTrue)
{
	ASSERT_TRUE(Contains("Lorem Ipsum", "Lorem Ipsum"));
}


TEST(StringUtilsTest, StartsWith)
{
	ASSERT_TRUE(StartsWith("abc", "abc"));
	ASSERT_TRUE(StartsWith("abc", "ab"));
	ASSERT_TRUE(StartsWith("abc", "a"));

	ASSERT_TRUE(!StartsWith("abc", "abcd"));
	ASSERT_TRUE(!StartsWith("abc", "bc"));
	ASSERT_TRUE(!StartsWith("abc", "c"));

	ASSERT_TRUE(!StartsWith("abc", "ABC"));
}


TEST(StringUtilsTest, EndsWith)
{
	ASSERT_TRUE(EndsWith("abc", "abc"));
	ASSERT_TRUE(EndsWith("abc", "bc"));
	ASSERT_TRUE(EndsWith("abc", "c"));

	ASSERT_TRUE(!EndsWith("abc", "abcd"));
	ASSERT_TRUE(!EndsWith("abc", "ab"));
	ASSERT_TRUE(!EndsWith("abc", "a"));

	ASSERT_TRUE(!EndsWith("abc", "ABC"));
}


TEST(StringUtilsTest, ExtractPrefix)
{
	ASSERT_EQ(ExtractPrefix("", 10), "");
	ASSERT_EQ(ExtractPrefix("abcde", 10), "abcde");
	ASSERT_EQ(ExtractPrefix("abcde", 5), "abcde");
	ASSERT_EQ(ExtractPrefix("abcde", 4), "abcd");
	ASSERT_EQ(ExtractPrefix("abcde", 3), "abc");
	ASSERT_EQ(ExtractPrefix("abcde", 1), "a");
	ASSERT_EQ(ExtractPrefix("abcde", 0), "");
}


TEST(StringUtilsTest, ExtractSuffix)
{
	ASSERT_EQ(ExtractSuffix("", 10), "");
	ASSERT_EQ(ExtractSuffix("abcde", 10), "abcde");
	ASSERT_EQ(ExtractSuffix("abcde", 5), "abcde");
	ASSERT_EQ(ExtractSuffix("abcde", 4), "bcde");
	ASSERT_EQ(ExtractSuffix("abcde", 3), "cde");
	ASSERT_EQ(ExtractSuffix("abcde", 1), "e");
	ASSERT_EQ(ExtractSuffix("abcde", 0), "");
}


TEST(StringUtilsTest, ExtractCommonPrefix)
{
	ASSERT_EQ(ExtractCommonPrefix("", ""), "");
	ASSERT_EQ(ExtractCommonPrefix("abcde", "abcde"), "abcde");
	ASSERT_EQ(ExtractCommonPrefix("abcde", "abcdf"), "abcd");
	ASSERT_EQ(ExtractCommonPrefix("abcde", "abcfg"), "abc");
	ASSERT_EQ(ExtractCommonPrefix("abcde", "afghi"), "a");
	ASSERT_EQ(ExtractCommonPrefix("abcde", "fghij"), "");
	ASSERT_EQ(ExtractCommonPrefix("abcde", "abcd"), "abcd");
	ASSERT_EQ(ExtractCommonPrefix("abcd", "abcde"), "abcd");
	ASSERT_EQ(ExtractCommonPrefix("abcde", "abf"), "ab");
	ASSERT_EQ(ExtractCommonPrefix("abf", "abcde"), "ab");
	ASSERT_EQ(ExtractCommonPrefix("abcde", "af"), "a");
	ASSERT_EQ(ExtractCommonPrefix("af", "abcde"), "a");
}


TEST(StringUtilsTest, ExtractCommonSuffix)
{
	ASSERT_EQ(ExtractCommonSuffix("", ""), "");
	ASSERT_EQ(ExtractCommonSuffix("abcde", "abcde"), "abcde");
	ASSERT_EQ(ExtractCommonSuffix("abcde", "zbcde"), "bcde");
	ASSERT_EQ(ExtractCommonSuffix("abcde", "yzcde"), "cde");
	ASSERT_EQ(ExtractCommonSuffix("abcde", "wxyze"), "e");
	ASSERT_EQ(ExtractCommonSuffix("abcde", "vwxyz"), "");
	ASSERT_EQ(ExtractCommonSuffix("abcde", "bcde"), "bcde");
	ASSERT_EQ(ExtractCommonSuffix("bcde", "abcde"), "bcde");
	ASSERT_EQ(ExtractCommonSuffix("abcde", "zde"), "de");
	ASSERT_EQ(ExtractCommonSuffix("zde", "abcde"), "de");
	ASSERT_EQ(ExtractCommonSuffix("abcde", "ze"), "e");
	ASSERT_EQ(ExtractCommonSuffix("ze", "abcde"), "e");
}


TEST(StringUtilsTest, RemovePrefix)
{
	ASSERT_EQ(RemovePrefix("", ""), "");
	ASSERT_EQ(RemovePrefix("abcde", "abcde"), "");
	ASSERT_EQ(RemovePrefix("abcde", "abcd"), "e");
	ASSERT_EQ(RemovePrefix("abcde", "abc"), "de");
	ASSERT_EQ(RemovePrefix("abcde", "a"), "bcde");
	ASSERT_EQ(RemovePrefix("abcde", ""), "abcde");
	ASSERT_EQ(RemovePrefix("abcde", "abcdf"), "abcde");
	ASSERT_EQ(RemovePrefix("abcde", "edcba"), "abcde");
}


TEST(StringUtilsTest, RemoveSuffix)
{
	ASSERT_EQ(RemoveSuffix("", ""), "");
	ASSERT_EQ(RemoveSuffix("abcde", "abcde"), "");
	ASSERT_EQ(RemoveSuffix("abcde", "bcde"), "a");
	ASSERT_EQ(RemoveSuffix("abcde", "cde"), "ab");
	ASSERT_EQ(RemoveSuffix("abcde", "e"), "abcd");
	ASSERT_EQ(RemoveSuffix("abcde", ""), "abcde");
	ASSERT_EQ(RemoveSuffix("abcde", "zbcde"), "abcde");
	ASSERT_EQ(RemoveSuffix("abcde", "edcba"), "abcde");
}


TEST(StringUtilsTest, Split)
{
	ASSERT_THAT(Split("/a//b/", "/"), MatchRange(ElementsAre("", "a", "", "b", "")));
	ASSERT_THAT(Split("ab//cd/e/f/.g/.", "/", 5), MatchRange(ElementsAre("ab", "", "cd", "e", "f", ".g/.")));
	ASSERT_THAT(Split("a/b/c/d", "/", 1), MatchRange(ElementsAre("a", "b/c/d")));
	ASSERT_THAT(Split("a,b.c;d", IsAnyOf(",.;"), 5), MatchRange(ElementsAre("a", "b", "c", "d")));
	ASSERT_THAT(Split("", "/"), MatchRange(ElementsAre("")));

	std::string a, b;
	int c;
	TupleFromStrings(ForwardAsTuple(a, b, c), Split("hello world 22", " "));
	ASSERT_EQ(a, "hello");
	ASSERT_EQ(b, "world");
	ASSERT_EQ(c, 22);

	ASSERT_ANY_THROW(TupleFromStrings(ForwardAsTuple(a, b, c), Split("hello world", " ")));

	ASSERT_ANY_THROW(TupleFromStrings(ForwardAsTuple(a, b, c), Split("hello world 22 33", " ")));

	{
		auto testee = Split("aaa|bbb|ccc", "|");
		ASSERT_THAT(testee, MatchRange(ElementsAre("aaa", "bbb", "ccc")));

		while (testee.Valid())
			testee.Next();
		ASSERT_THAT(testee, MatchRange(IsEmpty()));

		testee.First();
		ASSERT_THAT(testee, MatchRange(ElementsAre("aaa", "bbb", "ccc")));
	}
}


namespace { int IntAbs(int a) { return std::abs(a); } }


TEST(StringUtilsTest, OutputRange)
{
	std::vector<int> data(2);
	data[0] = 42;
	data[1] = -42;
	int a, b;
	TupleFromStrings(ForwardAsTuple(a, b), ToRange(data));
	ASSERT_EQ(a,  42);
	ASSERT_EQ(b, -42);

	TupleFromStrings(ForwardAsTuple(a, b), ToRange(data) | Transform(&IntAbs));
	ASSERT_EQ(a, 42);
	ASSERT_EQ(b, 42);
}


TEST(StringUtilsTest, Strip)
{
	const std::string str1 = "  test  ";

	ASSERT_EQ(LeftStrip(str1), "test  ");
	ASSERT_EQ(RightStrip(str1), "  test");
	ASSERT_EQ(Strip(str1), "test");

	const std::string str2 = "test";

	ASSERT_EQ(LeftStrip(str2), "test");
	ASSERT_EQ(RightStrip(str2), "test");
	ASSERT_EQ(Strip(str2), "test");

	const std::string str3 = "      ";

	ASSERT_EQ(LeftStrip(str3), "");
	ASSERT_EQ(RightStrip(str3), "");
	ASSERT_EQ(Strip(str3), "");
}


TEST(StringUtilsTest, ReplaceAll)
{
	const std::string str1 = "aabbccabc";
	const std::string str2 = str1, str3 = str1, str4 = str1;

	ASSERT_EQ(ReplaceAll(str2, "a", "1"), "11bbcc1bc");
	ASSERT_EQ(ReplaceAll(str3, "a", ""), "bbccbc");
	ASSERT_EQ(ReplaceAll(str4, "aa", ""), "bbccabc");
}


TEST(StringUtilsTest, Filter)
{
	const std::string str = "1|2/3|4/5*6*";

	ASSERT_EQ(Filter(str, ":"), "1|2/3|4/5*6*");
	ASSERT_EQ(Filter(str, "|"), "12/34/5*6*");
	ASSERT_EQ(Filter(str, "|/"), "12345*6*");
	ASSERT_EQ(Filter(str, "|/*"), "123456");
}


TEST(StringUtilsTest, ToLower)
{
	ASSERT_EQ(ToLower("a1c*e"), "a1c*e");
	ASSERT_EQ(ToLower("A1C*E"), "a1c*e");
	ASSERT_EQ(ToLower("A1C*E"), "a1c*e");
}


TEST(StringUtilsTest, ToUpper)
{
	ASSERT_EQ(ToUpper("A1C*E"), "A1C*E");
	ASSERT_EQ(ToUpper("a1c*e"), "A1C*E");
	ASSERT_EQ(ToUpper("A1C*E"), "A1C*E");
}


namespace
{

	static void AhoCallback(const std::string &pattern, size_t patternIndex, size_t offset, int &counter)
	{
		if (pattern == "aho")
		{
			ASSERT_EQ(patternIndex, 0u);
			ASSERT_EQ(offset, 0u);
		}
		else if (pattern == "ahocor")
		{
			ASSERT_EQ(patternIndex, 1u);
			ASSERT_EQ(offset, 0u);
		}
		else if (pattern == "cor")
		{
			ASSERT_EQ(patternIndex, 2u);
			ASSERT_EQ(offset, 3u);
		}
		else if (pattern == "sick")
		{
			ASSERT_EQ(patternIndex, 3u);
			ASSERT_EQ(offset, 7u);
		}
		++counter;
	}

}


TEST(StringUtilsTest, Aho)
{
	AhoCorasick aho;
	aho.Add("aho");
	aho.Add("ahocor");
	aho.Add("cor");
	aho.Add("sick");
	aho.Build();
	int matches = 0;
	aho.Search(std::string("ahocorasick"), Bind(&AhoCallback, _1, _2, _3, wrap_ref(matches)));
	ASSERT_EQ(matches, 4);
}


TEST(StringUtilsTest, ToStringIntegers)
{
	ASSERT_EQ(ToString((u8)0), "0");
	ASSERT_EQ(ToString((u8)255), "255");

	ASSERT_EQ(ToString((u16)0), "0");
	ASSERT_EQ(ToString((u16)65535), "65535");

	ASSERT_EQ(ToString((s16)-32767), "-32767");
	ASSERT_EQ(ToString((s16)0), "0");
	ASSERT_EQ(ToString((s16)32767), "32767");

	ASSERT_EQ(ToString((u32)0), "0");
	ASSERT_EQ(ToString((u32)4294967295u), "4294967295");

	ASSERT_EQ(ToString((s32)-2147483647), "-2147483647");
	ASSERT_EQ(ToString((s32)0), "0");
	ASSERT_EQ(ToString((s32)2147483647), "2147483647");

	ASSERT_EQ(ToString(9223372036854775807ll), "9223372036854775807");
	ASSERT_EQ(ToString(-9223372036854775807ll), "-9223372036854775807");
	ASSERT_EQ(ToString(18446744073709551615ull), "18446744073709551615");
}


TEST(StringUtilsTest, Join)
{
	std::vector<u8> ip = { 224, 0, 0, 251 };
	ASSERT_EQ(Join(".", ip.begin(), ip.end()), "224.0.0.251");

	std::vector<u8> mac = { 0x01, 0x00, 0x5E, 0x00, 0x00, 0xFB };
	ASSERT_EQ(Join(":", mac.begin(), mac.end(), Bind(&ToHex<u8>, _1, 2 * sizeof(u8), false, false)), "01:00:5e:00:00:fb");
}


TEST(StringUtilsTest, Capitalize)
{
	ASSERT_TRUE(Capitalize("") == "");
	ASSERT_TRUE(Capitalize("faggot") == "Faggot");
	ASSERT_TRUE(Capitalize("Faggot") == "Faggot");
}


TEST(StringUtilsTest, Uncapitalize)
{
	ASSERT_TRUE(Uncapitalize("") == "");
	ASSERT_TRUE(Uncapitalize("Faggot") == "faggot");
	ASSERT_TRUE(Uncapitalize("faggot") == "faggot");
}
