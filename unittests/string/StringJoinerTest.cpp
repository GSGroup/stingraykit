#include <stingraykit/string/ToString.h>

#include <gtest/gtest.h>

using namespace stingray;

TEST(StringJoinerTest, Join)
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

	ASSERT_EQ(ToString(StringJoiner("", "{", "}") % "a" % "b" % "c"), "{abc}");
	ASSERT_EQ(ToString(StringJoiner("|", "{", "}") % "a" % "b" % "c"), "{a|b|c}");
}
