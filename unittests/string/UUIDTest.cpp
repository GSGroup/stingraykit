// Copyright (c) 2011 - 2024, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/exception.h>
#include <stingraykit/UUID.h>

#include <gtest/gtest.h>

using namespace stingray;

TEST(UUIDTest, FromStringWithHyphens)
{
	const u8 uuid[] = { 0x53, 0xfb, 0x5b, 0xf0, 0x6a, 0xcd, 0x4b, 0xb4, 0x82, 0xe9, 0x00, 0x14, 0x3f, 0xd7, 0xc0, 0x26 };
	ASSERT_EQ(UUID::FromString("53fb5bf0-6acd-4bb4-82e9-00143fd7c026"), UUID(uuid));
}


TEST(UUIDTest, LongerThanNeeded)
{ ASSERT_THROW(UUID::FromString("53fb5bf0-6acd-4bb4-82e9-00143fd7c0267"), FormatException); }


TEST(UUIDTest, ShorterThanNeeded)
{ ASSERT_THROW(UUID::FromString("53fb5bf0-6acd-4bb4-82e9-00143fd7c02"), FormatException); }


TEST(UUIDTest, TrailingExtra)
{ ASSERT_THROW(UUID::FromString("53fb5bf0-6acd-4bb4-82e9-00143fd7c026  "), FormatException); }


TEST(UUIDTest, LeadingExtra)
{ ASSERT_THROW(UUID::FromString("  53fb5bf0-6acd-4bb4-82e9-00143fd7c026"), FormatException); }


TEST(UUIDTest, WrongAlpha)
{ ASSERT_THROW(UUID::FromString("53fb5bf0-6Zcd-4bb4-82e9-00143fd7c026"), FormatException); }


TEST(UUIDTest, MisplacedHyphen)
{ ASSERT_THROW(UUID::FromString("53fb5bf06acd-4b-b4-82e9-00143fd7c026"), FormatException); }


TEST(UUIDTest, ExtraHyphen)
{ ASSERT_THROW(UUID::FromString("53fb-5bf0-6acd-4bb4-82e9-00143fd7c026"), FormatException); }


TEST(UUIDTest, MissedHyphen)
{ ASSERT_THROW(UUID::FromString("53fb5bf0-6acd-4bb482e9-00143fd7c026"), FormatException); }


TEST(UUIDTest, Consistency)
{
	const std::string str = "53fb5bf0-6acd-4bb4-82e9-00143fd7c026";
	ASSERT_EQ(UUID::FromString(str).ToString(), str);
}


TEST(UUIDTest, VersionAndVariant)
{
	const u8 uuid[] = { 0x53, 0xfb, 0x5b, 0xf0, 0x6a, 0xcd, 0xff, 0xff, 0xff, 0xff, 0x00, 0x14, 0x3f, 0xd7, 0xc0, 0x26 };
	ASSERT_EQ(UUID(uuid, true).ToString(), "53fb5bf0-6acd-4fff-bfff-00143fd7c026");
}
