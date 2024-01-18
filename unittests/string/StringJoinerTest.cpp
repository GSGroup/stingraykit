// Copyright (c) 2011 - 2024, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

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
