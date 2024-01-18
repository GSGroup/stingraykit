// Copyright (c) 2011 - 2024, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/log/LogLevel.h>
#include <stingraykit/string/StringParse.h>
#include <stingraykit/string/StringUtils.h>

#include <gtest/gtest.h>

using namespace stingray;


TEST(StringParseTest, Medley)
{
	char c;
	int i;
	double d;
	std::string s;

	ASSERT_TRUE(StringParse("character: z, integer: 100500, double: 3.14, string: Universe", "character: %1%, integer: %2%, double: %3%, string: %4%", c, i, d, s));
	ASSERT_EQ(c, 'z');
	ASSERT_EQ(i, 100500);
	ASSERT_DOUBLE_EQ(d, 3.14);
	ASSERT_EQ(s, "Universe");

	ASSERT_TRUE(StringParse("Go home, %username%, you're drunk", "Go home, %username%, you're %1%", s));
	ASSERT_EQ(s, "drunk");

	ASSERT_TRUE(!StringParse("not integer: Universe", "not integer: %1%", i));

	ASSERT_ANY_THROW(StringParse("1 2", "%1% %2%", i));

	std::string method, uri, version;
	ASSERT_TRUE(StringParse("POST /keys HTTP/1.1", "%1% %2% %3%", method, uri, version));
	ASSERT_EQ(method, "POST");
	ASSERT_EQ(uri, "/keys");
	ASSERT_EQ(version, "HTTP/1.1");

	std::string name, value;
	ASSERT_TRUE(StringParse("Accept: text/plain; application/json", "%1%: %2%", name, value));
	ASSERT_EQ(name, "Accept");
	ASSERT_EQ(value, "text/plain; application/json");

	std::string filename;
	int part_no;
	ASSERT_FALSE(StringParse("video.avi.part", "%1%.part%2%", filename, part_no));
	ASSERT_FALSE(StringParse("video.avi.part1.dat", "%1%.part%2%", filename, part_no));
	ASSERT_TRUE(StringParse("video.avi.part1", "%1%.part%2%", filename, part_no));
	ASSERT_EQ(filename, "video.avi");
	ASSERT_EQ(part_no, 1);
}


namespace
{
	struct LogLevelConverter
	{
		static LogLevel Do(const std::string& str)
		{ return LogLevel::FromString(Capitalize(str)); }
	};
}


TEST(StringParseTest, Enum)
{
	LogLevel loglevel;
	ASSERT_TRUE(StringParse("Log level is Debug", "Log level is %1%", loglevel));
	ASSERT_EQ(loglevel, LogLevel::Debug);

	ASSERT_TRUE(StringParse("Log level is trace", "Log level is %1%", *MakeParseProxy(loglevel, &LogLevelConverter::Do)));
	ASSERT_EQ(loglevel, LogLevel::Trace);
}


TEST(StringParseTest, Mixture)
{
	ASSERT_TRUE(StringParse("p32p1", "p%_%p%_%"));

	int val = 0;
	ASSERT_TRUE(StringParse("1t", "%1%t", val));
	ASSERT_EQ(val, 1);
}


TEST(StringParseTest, Optional)
{
	optional<int> optI = 0;
	optional<int> optIn = null;
	ASSERT_TRUE(StringParse("honor my 1st rule and 2nd rule", "honor my %1%st rule and %2%nd rule", optI, optIn));
	ASSERT_EQ(*optI, 1);
	ASSERT_EQ(*optIn, 2);

	optional<LogLevel> optLoglevel;
	ASSERT_TRUE(StringParse("Log level is Debug", "Log level is %1%", optLoglevel));
	ASSERT_EQ(*optLoglevel, LogLevel::Debug);

	optional<char> optc;
	optional<u8> optu;
	optional<double> optd;
	optional<std::string> opts;

	ASSERT_TRUE(StringParse("char: z, u8: 15, double: 3.14, string: Universe", "char: %1%, u8: %2%, double: %3%, string: %4%", optc, optu, optd, opts));
	ASSERT_EQ(*optc, 'z');
	ASSERT_EQ(*optu, 15);
	ASSERT_DOUBLE_EQ(*optd, 3.14);
	ASSERT_EQ(*opts, "Universe");
}


TEST(StringParseTest, Integers)
{
	int x;
	ASSERT_FALSE(StringParse("asd", "%1%", x));

	u8 u1;
	EXPECT_TRUE(StringParse("255", "%1%", u1));
	EXPECT_EQ(u1, 255);
	EXPECT_FALSE(StringParse("256", "%1%", u1));
	EXPECT_FALSE(StringParse("-1", "%1%", u1));

	s8 s1;
	EXPECT_TRUE(StringParse("127", "%1%", s1));
	EXPECT_EQ(s1, 127);
	EXPECT_TRUE(StringParse("-128", "%1%", s1));
	EXPECT_EQ(s1, -128);
	EXPECT_FALSE(StringParse("128", "%1%", s1));
	EXPECT_FALSE(StringParse("-129", "%1%", s1));

	u16 u2;
	EXPECT_TRUE(StringParse("65535", "%1%", u2));
	EXPECT_EQ(u2, 65535);
	EXPECT_FALSE(StringParse("65536", "%1%", u2));
	EXPECT_FALSE(StringParse("-1", "%1%", u2));

	s16 s2;
	EXPECT_TRUE(StringParse("32767", "%1%", s2));
	EXPECT_EQ(s2, 32767);
	EXPECT_TRUE(StringParse("-32768", "%1%", s2));
	EXPECT_EQ(s2, -32768);
	EXPECT_FALSE(StringParse("32768", "%1%", s2));
	EXPECT_FALSE(StringParse("-32769", "%1%", s2));

	u32 u3;
	EXPECT_TRUE(StringParse("4294967295", "%1%", u3));
	EXPECT_EQ(u3, 4294967295ull);
	EXPECT_FALSE(StringParse("4294967296", "%1%", u3));
	EXPECT_FALSE(StringParse("-1", "%1%", u3));

	s32 s3;
	EXPECT_TRUE(StringParse("2147483647", "%1%", s3));
	EXPECT_EQ(s3, 2147483647);
	EXPECT_TRUE(StringParse("-2147483648", "%1%", s3));
	EXPECT_EQ(s3, -2147483648ll);
	EXPECT_FALSE(StringParse("2147483648", "%1%", s3));
	EXPECT_FALSE(StringParse("-2147483649", "%1%", s3));
}
