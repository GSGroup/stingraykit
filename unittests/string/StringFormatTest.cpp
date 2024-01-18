// Copyright (c) 2011 - 2024, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/string/StringFormat.h>

#include <gtest/gtest.h>

using namespace stingray;

TEST(StringFormatTest, Format)
{
	ASSERT_EQ(StringFormat("Format %1% %2% %3%", 1, 2, 3), "Format 1 2 3");
	ASSERT_EQ(StringFormat("Format %1% %2% %3%", "a", "b", "c"), "Format a b c");
	ASSERT_EQ(StringFormat("Format %3% a %2%", "a", "b", "c"), "Format c a b");
	ASSERT_EQ(StringFormat("Format %1% %% %3%", "a", "b", "c"), "Format a % c");
	ASSERT_EQ(StringFormat("Format %1% %% %3%", "a", "b", "c"), "Format a % c");
	ASSERT_ANY_THROW(StringFormat("Format %1% % %3%", "a", "b", "c"));
	ASSERT_ANY_THROW(StringFormat("Format %1% % % %3%", "a", "b", "c"));
	ASSERT_ANY_THROW(StringFormat("Format %1% %abc% %3%", "a", "b", "c"));
	ASSERT_ANY_THROW(StringFormat("Format %1% %1c% %3%", "a", "b", "c"));
	ASSERT_EQ(StringFormat("Format %1$1% %2$2% %3$5%", 1, 2, 3), "Format 1 02 00003");
}
