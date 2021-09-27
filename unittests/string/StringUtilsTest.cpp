#include <stingraykit/function/bind.h>
#include <stingraykit/string/AhoCorasick.h>
#include <stingraykit/string/Hex.h>
#include <stingraykit/string/StringUtils.h>
#include <stingraykit/PointerTupleBuilder.h>

#include <gmock/gmock-matchers.h>

using namespace stingray;

using ::testing::ElementsAre;

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


TEST(StringUtilsTest, Split)
{
	std::vector<std::string> result;

	Copy(Split("/a//b/", "/"), std::back_inserter(result));
	ASSERT_THAT(result, ElementsAre("", "a", "", "b", ""));

	result.clear();

	Copy(Split("ab//cd/e/f/.g/.", "/", 5), std::back_inserter(result));
	ASSERT_THAT(result, ElementsAre("ab", "", "cd", "e", "f", ".g/."));

	result.clear();

	Copy(Split("a/b/c/d", "/", 1), std::back_inserter(result));
	ASSERT_THAT(result, ElementsAre("a", "b/c/d"));

	result.clear();

	Copy(Split("a,b.c;d", IsAnyOf(",.;"), 5), std::back_inserter(result));
	ASSERT_THAT(result, ElementsAre("a", "b", "c", "d"));

	std::string a, b;
	int c;
	TupleFromStrings((PointerTupleBuilder() % a % b % c).Get(), Split("hello world 22", " "));
	ASSERT_EQ(a, "hello");
	ASSERT_EQ(b, "world");
	ASSERT_EQ(c, 22);
}


namespace { int IntAbs(int a) { return std::abs(a); } }


TEST(StringUtilsTest, OutputRange)
{
	std::vector<int> data(2);
	data[0] = 42;
	data[1] = -42;
	int a, b;
	TupleFromStrings((PointerTupleBuilder() % a % b).Get(), ToRange(data));
	ASSERT_EQ(a,  42);
	ASSERT_EQ(b, -42);

	TupleFromStrings((PointerTupleBuilder() % a % b).Get(), ToRange(data) | Transform(&IntAbs));
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
	std::string str1 = "aabbccabc";
	std::string str2 = str1, str3 = str1, str4 = str1;

	ASSERT_EQ(ReplaceAll(str2, "a", "1"), "11bbcc1bc");
	ASSERT_EQ(ReplaceAll(str3, "a", ""), "bbccbc");
	ASSERT_EQ(ReplaceAll(str4, "aa", ""), "bbccabc");
}


TEST(StringUtilsTest, Filter)
{
	std::string str = "1|2/3|4/5*6*";

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


TEST(StringUtilsTest, ToHex)
{
	ASSERT_EQ(ToHex(0), "0");
	ASSERT_EQ(ToHex(0, 10), "0000000000");
	ASSERT_EQ(ToHex<u16>(0xff, 10), "00000000ff");
	ASSERT_EQ(ToHex(0xffffffu), "ffffff");
	ASSERT_EQ(ToHex(0xffffffu, 0, true), "FFFFFF");
	ASSERT_EQ(ToHex(0xffffffu, 0, true, true), "0xFFFFFF");
	ASSERT_EQ(ToHex(0x12345678u, 0, true, true), "0x12345678");
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
