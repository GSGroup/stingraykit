// Copyright (c) 2011 - 2025, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/string/Hex.h>

#include <gmock/gmock-more-matchers.h>

using namespace stingray;

using ::testing::ElementsAre;
using ::testing::IsEmpty;

TEST(HexTest, IntegerToHex)
{
	ASSERT_EQ(ToHex(0), "0");
	ASSERT_EQ(ToHex(1), "1");
	ASSERT_EQ(ToHex(2), "2");
	ASSERT_EQ(ToHex(3), "3");
	ASSERT_EQ(ToHex(4), "4");
	ASSERT_EQ(ToHex(5), "5");
	ASSERT_EQ(ToHex(6), "6");
	ASSERT_EQ(ToHex(7), "7");
	ASSERT_EQ(ToHex(8), "8");
	ASSERT_EQ(ToHex(9), "9");
	ASSERT_EQ(ToHex(10), "a");
	ASSERT_EQ(ToHex(11), "b");
	ASSERT_EQ(ToHex(12), "c");
	ASSERT_EQ(ToHex(13), "d");
	ASSERT_EQ(ToHex(14), "e");
	ASSERT_EQ(ToHex(15), "f");
	ASSERT_EQ(ToHex(16), "10");

	ASSERT_EQ(ToHex(0, 2), "00");
	ASSERT_EQ(ToHex(1, 2), "01");
	ASSERT_EQ(ToHex(8, 2), "08");
	ASSERT_EQ(ToHex(9, 2), "09");
	ASSERT_EQ(ToHex(10, 2), "0a");
	ASSERT_EQ(ToHex(15, 2), "0f");
	ASSERT_EQ(ToHex(16, 2), "10");

	ASSERT_EQ(ToHex(0, 3), "000");
	ASSERT_EQ(ToHex(1, 3), "001");
	ASSERT_EQ(ToHex(8, 3), "008");
	ASSERT_EQ(ToHex(9, 3), "009");
	ASSERT_EQ(ToHex(10, 3), "00a");
	ASSERT_EQ(ToHex(15, 3), "00f");
	ASSERT_EQ(ToHex(16, 3), "010");

	ASSERT_EQ(ToHex(0, 4), "0000");
	ASSERT_EQ(ToHex(1, 4), "0001");
	ASSERT_EQ(ToHex(8, 4), "0008");
	ASSERT_EQ(ToHex(9, 4), "0009");
	ASSERT_EQ(ToHex(10, 4), "000a");
	ASSERT_EQ(ToHex(15, 4), "000f");
	ASSERT_EQ(ToHex(16, 4), "0010");

	ASSERT_EQ(ToHex(0, 5), "00000");
	ASSERT_EQ(ToHex(1, 5), "00001");
	ASSERT_EQ(ToHex(8, 5), "00008");
	ASSERT_EQ(ToHex(9, 5), "00009");
	ASSERT_EQ(ToHex(10, 5), "0000a");
	ASSERT_EQ(ToHex(15, 5), "0000f");
	ASSERT_EQ(ToHex(16, 5), "00010");

	ASSERT_EQ(ToHex(0, 6), "000000");
	ASSERT_EQ(ToHex(1, 6), "000001");
	ASSERT_EQ(ToHex(8, 6), "000008");
	ASSERT_EQ(ToHex(9, 6), "000009");
	ASSERT_EQ(ToHex(10, 6), "00000a");
	ASSERT_EQ(ToHex(15, 6), "00000f");
	ASSERT_EQ(ToHex(16, 6), "000010");

	ASSERT_EQ(ToHex(0, 7), "0000000");
	ASSERT_EQ(ToHex(1, 7), "0000001");
	ASSERT_EQ(ToHex(8, 7), "0000008");
	ASSERT_EQ(ToHex(9, 7), "0000009");
	ASSERT_EQ(ToHex(10, 7), "000000a");
	ASSERT_EQ(ToHex(15, 7), "000000f");
	ASSERT_EQ(ToHex(16, 7), "0000010");

	ASSERT_EQ(ToHex(0, 8), "00000000");
	ASSERT_EQ(ToHex(1, 8), "00000001");
	ASSERT_EQ(ToHex(8, 8), "00000008");
	ASSERT_EQ(ToHex(9, 8), "00000009");
	ASSERT_EQ(ToHex(10, 8), "0000000a");
	ASSERT_EQ(ToHex(15, 8), "0000000f");
	ASSERT_EQ(ToHex(16, 8), "00000010");

	ASSERT_EQ(ToHex((u8)0x12), "12");
	ASSERT_EQ(ToHex((u8)0x12, 2), "12");
	ASSERT_EQ(ToHex((u8)0x12, 4), "0012");
	ASSERT_EQ(ToHex((u8)0x12, 8), "00000012");

	ASSERT_EQ(ToHex((u16)0x1234), "1234");
	ASSERT_EQ(ToHex((u16)0x1234, 2), "1234");
	ASSERT_EQ(ToHex((u16)0x1234, 4), "1234");
	ASSERT_EQ(ToHex((u16)0x1234, 8), "00001234");

	ASSERT_EQ(ToHex((u32)0x12345678), "12345678");
	ASSERT_EQ(ToHex((u32)0x12345678, 2), "12345678");
	ASSERT_EQ(ToHex((u32)0x12345678, 4), "12345678");
	ASSERT_EQ(ToHex((u32)0x12345678, 8), "12345678");
	ASSERT_EQ(ToHex((u32)0x12345678, 16), "0000000012345678");

	ASSERT_EQ(ToHex((u64)0x12345678abcdef12), "12345678abcdef12");
	ASSERT_EQ(ToHex((u64)0x12345678abcdef12, 2), "12345678abcdef12");
	ASSERT_EQ(ToHex((u64)0x12345678abcdef12, 4), "12345678abcdef12");
	ASSERT_EQ(ToHex((u64)0x12345678abcdef12, 8), "12345678abcdef12");
	ASSERT_EQ(ToHex((u64)0x12345678abcdef12, 16), "12345678abcdef12");
	ASSERT_EQ(ToHex((u64)0x12345678abcdef12, 32), "000000000000000012345678abcdef12");

	ASSERT_EQ(ToHex(std::numeric_limits<u8>::max()), "ff");
	ASSERT_EQ(ToHex(std::numeric_limits<u16>::max()), "ffff");
	ASSERT_EQ(ToHex(std::numeric_limits<u32>::max()), "ffffffff");
	ASSERT_EQ(ToHex(std::numeric_limits<u64>::max()), "ffffffffffffffff");

	ASSERT_EQ(ToHex((u8)0xa1, 0, false, false), "a1");
	ASSERT_EQ(ToHex((u8)0xa1, 0, false, true), "0xa1");
	ASSERT_EQ(ToHex((u8)0xa1, 0, true, false), "A1");
	ASSERT_EQ(ToHex((u8)0xa1, 0, true, true), "0xA1");

	ASSERT_EQ(ToHex((u16)0xa1b2, 0, false, false), "a1b2");
	ASSERT_EQ(ToHex((u16)0xa1b2, 0, false, true), "0xa1b2");
	ASSERT_EQ(ToHex((u16)0xa1b2, 0, true, false), "A1B2");
	ASSERT_EQ(ToHex((u16)0xa1b2, 0, true, true), "0xA1B2");

	ASSERT_EQ(ToHex((u32)0xa1b2c3d4, 0, false, false), "a1b2c3d4");
	ASSERT_EQ(ToHex((u32)0xa1b2c3d4, 0, false, true), "0xa1b2c3d4");
	ASSERT_EQ(ToHex((u32)0xa1b2c3d4, 0, true, false), "A1B2C3D4");
	ASSERT_EQ(ToHex((u32)0xa1b2c3d4, 0, true, true), "0xA1B2C3D4");

	ASSERT_EQ(ToHex((u64)0xa1b2c3d4e5f6a7b8, 0, false, false), "a1b2c3d4e5f6a7b8");
	ASSERT_EQ(ToHex((u64)0xa1b2c3d4e5f6a7b8, 0, false, true), "0xa1b2c3d4e5f6a7b8");
	ASSERT_EQ(ToHex((u64)0xa1b2c3d4e5f6a7b8, 0, true, false), "A1B2C3D4E5F6A7B8");
	ASSERT_EQ(ToHex((u64)0xa1b2c3d4e5f6a7b8, 0, true, true), "0xA1B2C3D4E5F6A7B8");
}


TEST(HexTest, IntegerFromHex)
{
	ASSERT_THROW(FromHex<int>(""), ArgumentException);

	ASSERT_EQ(FromHex<int>("0"), 0);
	ASSERT_EQ(FromHex<int>("1"), 1);
	ASSERT_EQ(FromHex<int>("2"), 2);
	ASSERT_EQ(FromHex<int>("3"), 3);
	ASSERT_EQ(FromHex<int>("4"), 4);
	ASSERT_EQ(FromHex<int>("5"), 5);
	ASSERT_EQ(FromHex<int>("6"), 6);
	ASSERT_EQ(FromHex<int>("7"), 7);
	ASSERT_EQ(FromHex<int>("8"), 8);
	ASSERT_EQ(FromHex<int>("9"), 9);
	ASSERT_EQ(FromHex<int>("a"), 10);
	ASSERT_EQ(FromHex<int>("b"), 11);
	ASSERT_EQ(FromHex<int>("c"), 12);
	ASSERT_EQ(FromHex<int>("d"), 13);
	ASSERT_EQ(FromHex<int>("e"), 14);
	ASSERT_EQ(FromHex<int>("f"), 15);
	ASSERT_EQ(FromHex<int>("10"), 16);

	ASSERT_EQ(FromHex<int>("00"), 0);
	ASSERT_EQ(FromHex<int>("01"), 1);
	ASSERT_EQ(FromHex<int>("08"), 8);
	ASSERT_EQ(FromHex<int>("09"), 9);
	ASSERT_EQ(FromHex<int>("0a"), 10);
	ASSERT_EQ(FromHex<int>("0f"), 15);
	ASSERT_EQ(FromHex<int>("10"), 16);

	ASSERT_EQ(FromHex<int>("000"), 0);
	ASSERT_EQ(FromHex<int>("001"), 1);
	ASSERT_EQ(FromHex<int>("008"), 8);
	ASSERT_EQ(FromHex<int>("009"), 9);
	ASSERT_EQ(FromHex<int>("00a"), 10);
	ASSERT_EQ(FromHex<int>("00f"), 15);
	ASSERT_EQ(FromHex<int>("010"), 16);

	ASSERT_EQ(FromHex<int>("0000"), 0);
	ASSERT_EQ(FromHex<int>("0001"), 1);
	ASSERT_EQ(FromHex<int>("0008"), 8);
	ASSERT_EQ(FromHex<int>("0009"), 9);
	ASSERT_EQ(FromHex<int>("000a"), 10);
	ASSERT_EQ(FromHex<int>("000f"), 15);
	ASSERT_EQ(FromHex<int>("0010"), 16);

	ASSERT_EQ(FromHex<int>("00000"), 0);
	ASSERT_EQ(FromHex<int>("00001"), 1);
	ASSERT_EQ(FromHex<int>("00008"), 8);
	ASSERT_EQ(FromHex<int>("00009"), 9);
	ASSERT_EQ(FromHex<int>("0000a"), 10);
	ASSERT_EQ(FromHex<int>("0000f"), 15);
	ASSERT_EQ(FromHex<int>("00010"), 16);

	ASSERT_EQ(FromHex<int>("000000"), 0);
	ASSERT_EQ(FromHex<int>("000001"), 1);
	ASSERT_EQ(FromHex<int>("000008"), 8);
	ASSERT_EQ(FromHex<int>("000009"), 9);
	ASSERT_EQ(FromHex<int>("00000a"), 10);
	ASSERT_EQ(FromHex<int>("00000f"), 15);
	ASSERT_EQ(FromHex<int>("000010"), 16);

	ASSERT_EQ(FromHex<int>("0000000"), 0);
	ASSERT_EQ(FromHex<int>("0000001"), 1);
	ASSERT_EQ(FromHex<int>("0000008"), 8);
	ASSERT_EQ(FromHex<int>("0000009"), 9);
	ASSERT_EQ(FromHex<int>("000000a"), 10);
	ASSERT_EQ(FromHex<int>("000000f"), 15);
	ASSERT_EQ(FromHex<int>("0000010"), 16);

	ASSERT_EQ(FromHex<int>("00000000"), 0);
	ASSERT_EQ(FromHex<int>("00000001"), 1);
	ASSERT_EQ(FromHex<int>("00000008"), 8);
	ASSERT_EQ(FromHex<int>("00000009"), 9);
	ASSERT_EQ(FromHex<int>("0000000a"), 10);
	ASSERT_EQ(FromHex<int>("0000000f"), 15);
	ASSERT_EQ(FromHex<int>("00000010"), 16);

	ASSERT_EQ(FromHex<u8>("12"), 0x12);
	ASSERT_EQ(FromHex<u8>("0012"), 0x12);
	ASSERT_THROW(FromHex<u8>("1234"), IndexOutOfRangeException);
	ASSERT_EQ(FromHex<u8>("00000012"), 0x12);
	ASSERT_THROW(FromHex<u8>("12345678"), IndexOutOfRangeException);

	ASSERT_EQ(FromHex<u16>("1234"), 0x1234);
	ASSERT_EQ(FromHex<u16>("00001234"), 0x1234);
	ASSERT_THROW(FromHex<u16>("12345678"), IndexOutOfRangeException);

	ASSERT_EQ(FromHex<u32>("12345678"), 0x12345678);
	ASSERT_EQ(FromHex<u32>("0000000012345678"), 0x12345678);
	ASSERT_THROW(FromHex<u32>("12345678abcdef90"), IndexOutOfRangeException);

	ASSERT_EQ(FromHex<u64>("12345678abcdef90"), 0x12345678abcdef90);
	ASSERT_EQ(FromHex<u64>("000000000000000012345678abcdef90"), 0x12345678abcdef90);
	ASSERT_THROW(FromHex<u64>("12345678abcdef9090fedcba87654321"), IndexOutOfRangeException);

	ASSERT_EQ(FromHex<u8>("ff"), std::numeric_limits<u8>::max());
	ASSERT_EQ(FromHex<u16>("ffff"), std::numeric_limits<u16>::max());
	ASSERT_EQ(FromHex<u32>("ffffffff"), std::numeric_limits<u32>::max());
	ASSERT_EQ(FromHex<u64>("ffffffffffffffff"), std::numeric_limits<u64>::max());

	ASSERT_EQ(FromHex<u8>("a1"), 0xa1);
	ASSERT_EQ(FromHex<u8>("A1"), 0xA1);

	ASSERT_EQ(FromHex<u16>("a1b2"), 0xa1b2);
	ASSERT_EQ(FromHex<u16>("A1B2"), 0xA1B2);

	ASSERT_EQ(FromHex<u32>("a1b2c3d4"), 0xa1b2c3d4);
	ASSERT_EQ(FromHex<u32>("A1B2C3D4"), 0xA1B2C3D4);

	ASSERT_EQ(FromHex<u64>("a1b2c3d4e5f6a7b8"), 0xa1b2c3d4e5f6a7b8);
	ASSERT_EQ(FromHex<u64>("A1B2C3D4E5F6A7B8"), 0xA1B2C3D4E5F6A7B8);

	ASSERT_THROW(FromHex<u8>("abyz"), FormatException);
	ASSERT_THROW(FromHex<u8>("abcz"), FormatException);
	ASSERT_THROW(FromHex<u8>("ABYZ"), FormatException);
	ASSERT_THROW(FromHex<u8>("ABCZ"), FormatException);

	ASSERT_THROW(FromHex<u16>("abcdwxyz"), FormatException);
	ASSERT_THROW(FromHex<u16>("abcdexyz"), FormatException);
	ASSERT_THROW(FromHex<u16>("ABCDWXYZ"), FormatException);
	ASSERT_THROW(FromHex<u16>("ABCDEXYZ"), FormatException);

	ASSERT_THROW(FromHex<u32>("abcdef12stuvwxyz"), FormatException);
	ASSERT_THROW(FromHex<u32>("abcdef123tuvwxyz"), FormatException);
	ASSERT_THROW(FromHex<u32>("ABCDEF12STUVWXYZ"), FormatException);
	ASSERT_THROW(FromHex<u32>("ABCDEF123TUVWXYZ"), FormatException);

	ASSERT_THROW(FromHex<u64>("abcdef1234567890klmnopqrstuvwxyz"), FormatException);
	ASSERT_THROW(FromHex<u64>("abcdef1234567890almnopqrstuvwxyz"), FormatException);
	ASSERT_THROW(FromHex<u64>("ABCDEF1234567890LKMNOPQRSTUVWXYZ"), FormatException);
	ASSERT_THROW(FromHex<u64>("ABCDEF1234567890AKMNOPQRSTUVWXYZ"), FormatException);

	for (char ch = 'H'; ch <= 'Z'; ++ch)
		ASSERT_THROW(FromHex<int>(std::string(1, ch)), FormatException);

	for (char ch = 'h'; ch <= 'z'; ++ch)
		ASSERT_THROW(FromHex<int>(std::string(1, ch)), FormatException);
}


TEST(HexTest, ByteArrayToHex)
{
	const u8 EmptyData[] = { };
	const u8 SampleData[] =
	{
			0x12, 0x34, 0x56, 0x78, 0x90, 0xab, 0xcd, 0xef,
			0xf1, 0x23, 0x45, 0x67, 0x89, 0x0a, 0xbc, 0xde,
			0xfe, 0xdc, 0xba, 0x09, 0x87, 0x65, 0x43, 0x21,
			0xed, 0xcb, 0xa0, 0x98, 0x76, 0x54, 0x32, 0x1f
	};

	const ConstByteData EmptyByteData(EmptyData, sizeof(EmptyData));
	const ConstByteData SampleByteData(SampleData);

	ASSERT_EQ(ShortHexDump(EmptyData, sizeof(EmptyData)).ToString(), "{ }");
	ASSERT_EQ(ShortHexDump(EmptyByteData).ToString(), "{ }");

	ASSERT_EQ(ShortHexDump(SampleData, sizeof(SampleData), sizeof(SampleData) / 4).ToString(), "{ 12 34 56 78 90 ab cd ef ... }");
	ASSERT_EQ(ShortHexDump(SampleByteData, SampleByteData.size() / 4).ToString(), "{ 12 34 56 78 90 ab cd ef ... }");

	ASSERT_EQ(ShortHexDump(SampleData, sizeof(SampleData)).ToString(), "{ 12 34 56 78 90 ab cd ef f1 23 45 67 89 0a bc de ... }");
	ASSERT_EQ(ShortHexDump(SampleByteData).ToString(), "{ 12 34 56 78 90 ab cd ef f1 23 45 67 89 0a bc de ... }");

	ASSERT_EQ(ShortHexDump(SampleData, sizeof(SampleData), sizeof(SampleData) * 3 / 4).ToString(), "{ 12 34 56 78 90 ab cd ef f1 23 45 67 89 0a bc de fe dc ba 09 87 65 43 21 ... }");
	ASSERT_EQ(ShortHexDump(SampleByteData, SampleByteData.size() * 3 / 4).ToString(), "{ 12 34 56 78 90 ab cd ef f1 23 45 67 89 0a bc de fe dc ba 09 87 65 43 21 ... }");

	ASSERT_EQ(ShortHexDump(SampleData, sizeof(SampleData), sizeof(SampleData)).ToString(), "{ 12 34 56 78 90 ab cd ef f1 23 45 67 89 0a bc de fe dc ba 09 87 65 43 21 ed cb a0 98 76 54 32 1f }");
	ASSERT_EQ(ShortHexDump(SampleByteData, SampleByteData.size()).ToString(), "{ 12 34 56 78 90 ab cd ef f1 23 45 67 89 0a bc de fe dc ba 09 87 65 43 21 ed cb a0 98 76 54 32 1f }");

	ASSERT_EQ(ShortHexDump(SampleData, sizeof(SampleData), sizeof(SampleData) * 2).ToString(), "{ 12 34 56 78 90 ab cd ef f1 23 45 67 89 0a bc de fe dc ba 09 87 65 43 21 ed cb a0 98 76 54 32 1f }");
	ASSERT_EQ(ShortHexDump(SampleByteData, SampleByteData.size() * 2).ToString(), "{ 12 34 56 78 90 ab cd ef f1 23 45 67 89 0a bc de fe dc ba 09 87 65 43 21 ed cb a0 98 76 54 32 1f }");

	ASSERT_EQ(HexDump(EmptyData, sizeof(EmptyData)).ToString(), "");
	ASSERT_EQ(HexDump(EmptyData, sizeof(EmptyData)).ToString(), "");

	ASSERT_EQ(
			HexDump(SampleData, sizeof(SampleData) / 4).ToString(),
			"00000000: 12 34 56 78 90 ab cd ef                         .4Vx....");
	ASSERT_EQ(
			HexDump(ConstByteData(SampleByteData, 0, SampleByteData.size() / 4)).ToString(),
			"00000000: 12 34 56 78 90 ab cd ef                         .4Vx....");

	ASSERT_EQ(
			HexDump(SampleData, sizeof(SampleData) / 2).ToString(),
			"00000000: 12 34 56 78 90 ab cd ef f1 23 45 67 89 0a bc de .4Vx.....#Eg....");
	ASSERT_EQ(
			HexDump(ConstByteData(SampleByteData, 0, SampleByteData.size() / 2)).ToString(),
			"00000000: 12 34 56 78 90 ab cd ef f1 23 45 67 89 0a bc de .4Vx.....#Eg....");

	ASSERT_EQ(
			HexDump(SampleData, sizeof(SampleData) * 3 / 4).ToString(),
			"00000000: 12 34 56 78 90 ab cd ef f1 23 45 67 89 0a bc de .4Vx.....#Eg....\n"
			"00000010: fe dc ba 09 87 65 43 21                         .....eC!");
	ASSERT_EQ(
			HexDump(ConstByteData(SampleByteData, 0, SampleByteData.size() * 3 / 4)).ToString(),
			"00000000: 12 34 56 78 90 ab cd ef f1 23 45 67 89 0a bc de .4Vx.....#Eg....\n"
			"00000010: fe dc ba 09 87 65 43 21                         .....eC!");

	ASSERT_EQ(
			HexDump(SampleData, sizeof(SampleData)).ToString(),
			"00000000: 12 34 56 78 90 ab cd ef f1 23 45 67 89 0a bc de .4Vx.....#Eg....\n"
			"00000010: fe dc ba 09 87 65 43 21 ed cb a0 98 76 54 32 1f .....eC!....vT2.");
	ASSERT_EQ(
			HexDump(SampleByteData).ToString(),
			"00000000: 12 34 56 78 90 ab cd ef f1 23 45 67 89 0a bc de .4Vx.....#Eg....\n"
			"00000010: fe dc ba 09 87 65 43 21 ed cb a0 98 76 54 32 1f .....eC!....vT2.");

	ASSERT_EQ(
			HexDump(SampleData, sizeof(SampleData) / 4, 8).ToString(),
			"00000000: 12 34 56 78 90 ab cd ef .4Vx....");
	ASSERT_EQ(
			HexDump(ConstByteData(SampleByteData, 0, SampleByteData.size() / 4), 8).ToString(),
			"00000000: 12 34 56 78 90 ab cd ef .4Vx....");

	ASSERT_EQ(
			HexDump(SampleData, sizeof(SampleData) / 2, 8).ToString(),
			"00000000: 12 34 56 78 90 ab cd ef .4Vx....\n"
			"00000008: f1 23 45 67 89 0a bc de .#Eg....");
	ASSERT_EQ(
			HexDump(ConstByteData(SampleByteData, 0, SampleByteData.size() / 2), 8).ToString(),
			"00000000: 12 34 56 78 90 ab cd ef .4Vx....\n"
			"00000008: f1 23 45 67 89 0a bc de .#Eg....");

	ASSERT_EQ(
			HexDump(SampleData, sizeof(SampleData) * 3 / 4, 8).ToString(),
			"00000000: 12 34 56 78 90 ab cd ef .4Vx....\n"
			"00000008: f1 23 45 67 89 0a bc de .#Eg....\n"
			"00000010: fe dc ba 09 87 65 43 21 .....eC!");
	ASSERT_EQ(
			HexDump(ConstByteData(SampleByteData, 0, SampleByteData.size() * 3 / 4), 8).ToString(),
			"00000000: 12 34 56 78 90 ab cd ef .4Vx....\n"
			"00000008: f1 23 45 67 89 0a bc de .#Eg....\n"
			"00000010: fe dc ba 09 87 65 43 21 .....eC!");

	ASSERT_EQ(
			HexDump(SampleData, sizeof(SampleData), 8).ToString(),
			"00000000: 12 34 56 78 90 ab cd ef .4Vx....\n"
			"00000008: f1 23 45 67 89 0a bc de .#Eg....\n"
			"00000010: fe dc ba 09 87 65 43 21 .....eC!\n"
			"00000018: ed cb a0 98 76 54 32 1f ....vT2.");
	ASSERT_EQ(
			HexDump(SampleByteData, 8).ToString(),
			"00000000: 12 34 56 78 90 ab cd ef .4Vx....\n"
			"00000008: f1 23 45 67 89 0a bc de .#Eg....\n"
			"00000010: fe dc ba 09 87 65 43 21 .....eC!\n"
			"00000018: ed cb a0 98 76 54 32 1f ....vT2.");
}


TEST(HexTest, ByteArrayFromHex)
{
	ASSERT_THAT(FromHex<ConstByteArray>(""), IsEmpty());

	ASSERT_THAT(FromHex<ConstByteArray>("1234567890abcdef"), ElementsAre(0x12, 0x34, 0x56, 0x78, 0x90, 0xab, 0xcd, 0xef));
	ASSERT_THAT(FromHex<ConstByteArray>("0x1234567890abcdef"), ElementsAre(0x12, 0x34, 0x56, 0x78, 0x90, 0xab, 0xcd, 0xef));

	ASSERT_THAT(FromHex<ConstByteArray>("1234567890ABCDEF"), ElementsAre(0x12, 0x34, 0x56, 0x78, 0x90, 0xab, 0xcd, 0xef));
	ASSERT_THAT(FromHex<ConstByteArray>("0x1234567890ABCDEF"), ElementsAre(0x12, 0x34, 0x56, 0x78, 0x90, 0xab, 0xcd, 0xef));

	ASSERT_THAT(FromHex<ConstByteArray>("1234567890aBcDeF"), ElementsAre(0x12, 0x34, 0x56, 0x78, 0x90, 0xab, 0xcd, 0xef));
	ASSERT_THAT(FromHex<ConstByteArray>("0x1234567890aBcDeF"), ElementsAre(0x12, 0x34, 0x56, 0x78, 0x90, 0xab, 0xcd, 0xef));

	ASSERT_THAT(FromHex<ConstByteArray>("1234567890AbCdEf"), ElementsAre(0x12, 0x34, 0x56, 0x78, 0x90, 0xab, 0xcd, 0xef));
	ASSERT_THAT(FromHex<ConstByteArray>("0x1234567890AbCdEf"), ElementsAre(0x12, 0x34, 0x56, 0x78, 0x90, 0xab, 0xcd, 0xef));

	ASSERT_THAT(FromHex<ConstByteArray>("1234567890abcdef0"), ElementsAre(0x1, 0x23, 0x45, 0x67, 0x89, 0x0a, 0xbc, 0xde, 0xf0));
	ASSERT_THAT(FromHex<ConstByteArray>("0x1234567890abcdef0"), ElementsAre(0x1, 0x23, 0x45, 0x67, 0x89, 0x0a, 0xbc, 0xde, 0xf0));

	for (char ch = 'H'; ch <= 'Z'; ++ch)
		ASSERT_THROW(FromHex<ConstByteArray>(std::string(1, ch)), FormatException);

	for (char ch = 'h'; ch <= 'z'; ++ch)
		ASSERT_THROW(FromHex<ConstByteArray>(std::string(1, ch)), FormatException);
}
