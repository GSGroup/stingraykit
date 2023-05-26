#include <stingraykit/string/ToString.h>

#include <gtest/gtest.h>

using namespace stingray;

TEST(StringJoinerTest, JoinWithSeparator)
{
	ASSERT_EQ(ToString(StringJoiner("")), "");
	ASSERT_EQ(ToString(StringJoiner("|")), "");

	ASSERT_EQ(ToString(StringJoiner("", "{", "}")), "{}");
	ASSERT_EQ(ToString(StringJoiner("|", "{", "}")), "{}");

	ASSERT_EQ(ToString(StringJoiner("") % "a"), "a");
	ASSERT_EQ(ToString(StringJoiner("|") % "a"), "a");

	ASSERT_EQ(ToString(StringJoiner("", "{", "}") % "a"), "{a}");
	ASSERT_EQ(ToString(StringJoiner("|", "{", "}") % "a"), "{a}");

	ASSERT_EQ(ToString(StringJoiner("") % "a" % "b" % "c"), "abc");
	ASSERT_EQ(ToString(StringJoiner("|") % "a" % "b" % "c"), "a|b|c");

	ASSERT_EQ(ToString(StringJoiner("|") % Sep % "a" % "b" % "c"), "a|b|c");
	ASSERT_EQ(ToString(StringJoiner("|") % "a" % Sep % "b" % "c"), "a|b|c");
	ASSERT_EQ(ToString(StringJoiner("|") % "a" % "b" % Sep % "c"), "a|b|c");

	ASSERT_EQ(ToString(StringJoiner("|") % NoSep % "a" % "b" % "c"), "a|b|c");
	ASSERT_EQ(ToString(StringJoiner("|") % "a" % NoSep % "b" % "c"), "ab|c");
	ASSERT_EQ(ToString(StringJoiner("|") % "a" % "b" % NoSep % "c"), "a|bc");

	ASSERT_EQ(ToString(StringJoiner("", "{", "}") % "a" % "b" % "c"), "{abc}");
	ASSERT_EQ(ToString(StringJoiner("|", "{", "}") % "a" % "b" % "c"), "{a|b|c}");

	ASSERT_EQ(ToString(StringJoiner("|", "{", "}") % Sep % "a" % "b" % "c"), "{a|b|c}");
	ASSERT_EQ(ToString(StringJoiner("|", "{", "}") % "a" % Sep % "b" % "c"), "{a|b|c}");
	ASSERT_EQ(ToString(StringJoiner("|", "{", "}") % "a" % "b" % Sep % "c"), "{a|b|c}");

	ASSERT_EQ(ToString(StringJoiner("|", "{", "}") % NoSep % "a" % "b" % "c"), "{a|b|c}");
	ASSERT_EQ(ToString(StringJoiner("|", "{", "}") % "a" % NoSep % "b" % "c"), "{ab|c}");
	ASSERT_EQ(ToString(StringJoiner("|", "{", "}") % "a" % "b" % NoSep % "c"), "{a|bc}");
}

TEST(StringJoinerTest, JoinWithoutSeparator)
{
	ASSERT_EQ(ToString(StringJoiner("|", false)), "");

	ASSERT_EQ(ToString(StringJoiner("|", "{", "}", false)), "{}");

	ASSERT_EQ(ToString(StringJoiner("|", false) % "a"), "a");
	ASSERT_EQ(ToString(StringJoiner("|", false) % Sep % "a"), "a");
	ASSERT_EQ(ToString(StringJoiner("|", false) % NoSep % "a"), "a");

	ASSERT_EQ(ToString(StringJoiner("|", "{", "}", false) % "a"), "{a}");
	ASSERT_EQ(ToString(StringJoiner("|", "{", "}", false) % Sep % "a"), "{a}");
	ASSERT_EQ(ToString(StringJoiner("|", "{", "}", false) % NoSep % "a"), "{a}");

	ASSERT_EQ(ToString(StringJoiner("|", false) % "a" % "b" % "c"), "abc");

	ASSERT_EQ(ToString(StringJoiner("|", false) % Sep % "a" % "b" % "c"), "abc");
	ASSERT_EQ(ToString(StringJoiner("|", false) % "a" % Sep % "b" % "c"), "a|bc");
	ASSERT_EQ(ToString(StringJoiner("|", false) % "a" % "b" % Sep % "c"), "ab|c");

	ASSERT_EQ(ToString(StringJoiner("|", false) % NoSep % "a" % "b" % "c"), "abc");
	ASSERT_EQ(ToString(StringJoiner("|", false) % "a" % NoSep % "b" % "c"), "abc");
	ASSERT_EQ(ToString(StringJoiner("|", false) % "a" % "b" % NoSep % "c"), "abc");

	ASSERT_EQ(ToString(StringJoiner("|", "{", "}", false) % "a" % "b" % "c"), "{abc}");

	ASSERT_EQ(ToString(StringJoiner("|", "{", "}", false) % Sep % "a" % "b" % "c"), "{abc}");
	ASSERT_EQ(ToString(StringJoiner("|", "{", "}", false) % "a" % Sep % "b" % "c"), "{a|bc}");
	ASSERT_EQ(ToString(StringJoiner("|", "{", "}", false) % "a" % "b" % Sep % "c"), "{ab|c}");

	ASSERT_EQ(ToString(StringJoiner("|", "{", "}", false) % NoSep % "a" % "b" % "c"), "{abc}");
	ASSERT_EQ(ToString(StringJoiner("|", "{", "}", false) % "a" % NoSep % "b" % "c"), "{abc}");
	ASSERT_EQ(ToString(StringJoiner("|", "{", "}", false) % "a" % "b" % NoSep % "c"), "{abc}");
}
