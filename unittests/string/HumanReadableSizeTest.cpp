// Copyright (c) 2011 - 2024, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/string/HumanReadableSize.h>

#include <gtest/gtest.h>

using namespace stingray;

TEST(HumanReadableSize, ToHumanReadableSize)
{
	ASSERT_EQ(ToHumanReadableSize(0), "0");
	ASSERT_EQ(ToHumanReadableSize(1), "1");
	ASSERT_EQ(ToHumanReadableSize((u64)1 << 10), "1k");
	ASSERT_EQ(ToHumanReadableSize((u64)2 << 20), "2M");
	ASSERT_EQ(ToHumanReadableSize((u64)3 << 30), "3G");
	ASSERT_EQ(ToHumanReadableSize((u64)4 << 40), "4T");
	ASSERT_EQ(ToHumanReadableSize((u64)5 << 50), "5P");
	ASSERT_EQ(ToHumanReadableSize((u64)6 << 60), "6E");

	ASSERT_EQ(ToHumanReadableSize(153), "153");
	ASSERT_EQ(ToHumanReadableSize(1500), "1500");
	ASSERT_EQ(ToHumanReadableSize(2047), "2k");
	ASSERT_EQ(ToHumanReadableSize(2047, 0.0), "2047");
	ASSERT_EQ(ToHumanReadableSize(10000000), "9766k");
	ASSERT_EQ(ToHumanReadableSize(10000000, 0.0), "10000000");
	ASSERT_EQ(ToHumanReadableSize(((u64)1 << 20) - 10), "1M");
	ASSERT_EQ(ToHumanReadableSize(((u64)2 << 20) - 10), "2M");
	ASSERT_EQ(ToHumanReadableSize(((u64)2 << 20) - ((u64)2 << 20) / 50), "2007k");
	ASSERT_EQ(ToHumanReadableSize(((u64)123 << 30)), "123G");
}


TEST(HumanReadableSize, FromHumanReadableSize)
{
	ASSERT_EQ(FromHumanReadableSize("153"), (u64)153);
	ASSERT_EQ(FromHumanReadableSize("4343434"), (u64)4343434);
	ASSERT_EQ(FromHumanReadableSize("2k"), (u64)2 << 10);
	ASSERT_EQ(FromHumanReadableSize("76M"), (u64)76 << 20);
	ASSERT_EQ(FromHumanReadableSize("160G"), (u64)160 << 30);

	ASSERT_ANY_THROW(FromHumanReadableSize("123Z"));
}
