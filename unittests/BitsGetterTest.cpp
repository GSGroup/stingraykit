#include <stingraykit/collection/ByteData.h>
#include <stingraykit/BitsGetter.h>
#include <stingraykit/io/BitStream.h>

#include <gtest/gtest.h>

using namespace stingray;

TEST(BitsGetterTest, OutOfBoundsException)
{
	u8 test_data[4] = {0xff, 0xff, 0xff, 0xff};
	ConstByteData byte_data(test_data, sizeof(test_data));
	BitsGetter bits(byte_data);

	bits.Get<0, 32>(); //ok, full range
	u32 dummy;
	(void)dummy;
	ASSERT_ANY_THROW((dummy = bits.Get<1, 32>())); //one bit after
	ASSERT_ANY_THROW((dummy = bits.Get<31, 2>())); //one bit after

	// Due to special checks doesn't compile now
	//ASSERT_ANY_THROW(((u32)bits.Get<-1, 1>())); //one bit before
	//ASSERT_ANY_THROW(((u32)bits.Get<-8, 8>())); //one byte before

	ASSERT_ANY_THROW((dummy = bits.Get<32, 8>())); //one byte after
	ASSERT_ANY_THROW((dummy = bits.Get<64, 32>())); //far away
}


TEST(BitsGetterTest, BitStream)
{
	ByteArray ba;
	SeisedBitStream bs_out(ba);
	bs_out.Write<2>(0);
	bs_out.Write<14>(1);
	bs_out.Write<10>(1);
	bs_out.Write<6>(1);
	SeisedBitStream bs_in(ba);
	ASSERT_EQ((u32)bs_in.Read<32>(), 0x00010041u);
}


TEST(BitsGetterTest, BitsSetter)
{
	{
		std::vector<u8> vec(4);
		ByteData ba(vec);
		BitsSetter bs(ba);
		BitsGetter bg(ba);

		bs.Set<0, 2>(0);
		ASSERT_EQ((u32)(bg.Get<0, 32>()), 0x00000000u);
		bs.Set<2, 14>(1);
		ASSERT_EQ((u32)(bg.Get<0, 32>()), 0x00010000u);
		bs.Set<16, 10>(0x201);
		ASSERT_EQ((u32)(bg.Get<0, 32>()), 0x00018040u);
		bs.Set<26, 6>(1);
		ASSERT_EQ((u32)(bg.Get<0, 32>()), 0x00018041u);
	}
	{
		std::vector<u8> vec(2);
		ByteData ba(vec);
		BitsSetter bs(ba);
		BitsGetter bg(ba);
		bs.Set<0, 4>(0xFFFFFFFF);
		bs.Set<4, 12>(6);
		ASSERT_EQ((u32)(bg.Get<0, 16>()), 0xF006u);
	}
}


TEST(BitsGetterTest, Bits)
{
	u8 test_data[3] = {0xff, 0x00, 0xff};
	ConstByteData byte_data(test_data, sizeof(test_data));

	BitsGetter bits(byte_data);
	ASSERT_EQ((u8)(bits.Get<0, 1>()), 1);

	ASSERT_EQ((u8)(bits.Get<0, 4>()), 0x0f);
	ASSERT_EQ((u8)(bits.Get<1, 4>()), 0x0f);
	ASSERT_EQ((u8)(bits.Get<2, 4>()), 0x0f);
	ASSERT_EQ((u8)(bits.Get<3, 4>()), 0x0f);
	ASSERT_EQ((u8)(bits.Get<4, 4>()), 0x0f);

	ASSERT_EQ((u8)(bits.Get<5, 4>()), 0x0e);
	ASSERT_EQ((u8)(bits.Get<6, 4>()), 0x0c);
	ASSERT_EQ((u8)(bits.Get<7, 4>()), 0x08);
	ASSERT_EQ((u8)(bits.Get<8, 4>()), 0x00);

	ASSERT_EQ((u32)(bits.Get<0, 12>()), 0xff0u);
	ASSERT_EQ((u32)(bits.Get<0, 20>()), 0xff00fu);
	ASSERT_EQ((u32)(bits.Get<0, 24>()), 0xff00ffu);
	ASSERT_EQ((u32)(bits.Get<1, 11>()), 0x7f0u);
	ASSERT_EQ((u32)(bits.Get<16, 7>()), 0x7fu);
	ASSERT_EQ((u32)(bits.Get<16, 6>()), 0x3fu);
	ASSERT_EQ((u32)(bits.Get<16, 5>()), 0x1fu);
	ASSERT_EQ((u32)(bits.Get<16, 4>()), 0x0fu);
	ASSERT_EQ((u32)(bits.Get<16, 3>()), 0x07u);
	ASSERT_EQ((u32)(bits.Get<16, 2>()), 0x03u);
	ASSERT_EQ((u32)(bits.Get<16, 1>()), 0x01u);
	ASSERT_EQ((u32)(bits.Get<16, 0>()), 0x00u);
}


TEST(BitsGetterTest, Arithmetics)
{
	u8 buffer[] = { 0x11u, 0x22u, 0x33u, 0x44u, 0x55u, 0x66, 0x77, 0x88 };

	BitsGetter bits(buffer);

	ASSERT_EQ((bits.Get<0, 64>() + u32(1)), 0x1122334455667789u);
	ASSERT_EQ((bits.Get<0, 32>() + u16(1)), 0x11223345u);
	ASSERT_EQ((bits.Get<0, 16>() + u8(1)), 0x1123u);
	ASSERT_EQ((bits.Get<0, 8>() + true), 0x12u);
	ASSERT_EQ((bits.Get<0, 16>() + false), 0x1122u);
}


TEST(BitsGetterTest, Shifts)
{
	u8 buffer[] = { 0x11u, 0x22u, 0x33u, 0x44u, 0x55u, 0x66, 0x77, 0x88 };

	BitsGetter bits(buffer);

	ASSERT_EQ(((u64)bits.Get<0, 64>() << 8), 0x2233445566778800u);
	ASSERT_EQ(((u64)bits.Get<0, 48>() >> 8), 0x1122334455u);
}


TEST(BitsGetterTest, BigEndianBitsGetter)
{
	u8 buffer[] = { 0x11u, 0x22u, 0x33u, 0x44u, 0x55u, 0x66, 0x77 };

	BitsGetter bits(ConstByteData(buffer, sizeof(buffer)));

	ASSERT_EQ(( (u8)bits.Get< 0,  8>()), 0x11u);
	ASSERT_EQ(((u16)bits.Get< 8, 16>()), 0x2233u);
	ASSERT_EQ(((u32)bits.Get<24, 32>()), 0x44556677u);
}


TEST(BitsGetterTest, LittleEndianBitsGetter)
{
	u8 buffer[] = { 0x11u, 0x22u, 0x33u, 0x44u, 0x55u, 0x66, 0x77 };

	LittleEndianBitsGetter bits(ConstByteData(buffer, sizeof(buffer)));

	ASSERT_EQ(( (u8)bits.Get< 0,  8>()), 0x11u);
	ASSERT_EQ(((u32)bits.Get< 0, 24>()), 0x332211u);
	ASSERT_EQ(((u16)bits.Get< 8, 16>()), 0x3322u);
	ASSERT_EQ(((u32)bits.Get<24, 32>()), 0x77665544u);
}


TEST(BitsGetterTest, BigEndianBitsSetter)
{
	u8 buffer[sizeof(u8) + sizeof(u16) + sizeof(u32)];

	BitsSetter bits(ByteData(buffer, sizeof(buffer)));

	bits.Set< 0,  8>(0x11u);
	bits.Set< 8, 16>(0x2233u);
	bits.Set<24, 32>(0x44556677u);

	ASSERT_EQ(buffer[0], 0x11u);
	ASSERT_EQ(buffer[1], 0x22u);
	ASSERT_EQ(buffer[2], 0x33u);
	ASSERT_EQ(buffer[3], 0x44u);
	ASSERT_EQ(buffer[4], 0x55u);
	ASSERT_EQ(buffer[5], 0x66u);
	ASSERT_EQ(buffer[6], 0x77u);
}


TEST(BitsGetterTest, LittleEndianBitsSetter)
{
	u8 buffer[sizeof(u8) + sizeof(u16) + sizeof(u32)];

	LittleEndianBitsSetter bits(ByteData(buffer, sizeof(buffer)));

	bits.Set< 0,  8>(0x11u);
	bits.Set< 8, 16>(0x2233u);
	bits.Set<24, 32>(0x44556677u);

	ASSERT_EQ(buffer[0], 0x11u);
	ASSERT_EQ(buffer[1], 0x33u);
	ASSERT_EQ(buffer[2], 0x22u);
	ASSERT_EQ(buffer[3], 0x77u);
	ASSERT_EQ(buffer[4], 0x66u);
	ASSERT_EQ(buffer[5], 0x55u);
	ASSERT_EQ(buffer[6], 0x44u);

	u8 buffer2[] = { 0, 0 };

	LittleEndianBitsSetter bits2(ByteData(buffer2, sizeof(buffer2)));

	bits2.Set< 0, 2>(0x3u);
	bits2.Set< 5, 3>(0x7u);
	bits2.Set< 8, 1>(0x1u);
	bits2.Set<14, 2>(0x3u);

	ASSERT_EQ(buffer2[0], 0xc7u); //implementation does not reverse mbr bit order
	ASSERT_EQ(buffer2[1], 0x83u);
}


TEST(BitsGetterTest, BigEndianBitStreamRead)
{
	u8 buffer[] = { 0x11u, 0x22u, 0x33u, 0x44u, 0x55u, 0x66, 0x77 };

	ConstBitStream stream(ConstByteData(buffer, sizeof(buffer)));

	ASSERT_EQ(( (u8)stream.Read< 8>()), 0x11u);
	ASSERT_EQ(((u16)stream.Read<16>()), 0x2233u);
	ASSERT_EQ(((u32)stream.Read<32>()), 0x44556677u);
}


TEST(BitsGetterTest, LittleEndianBitStreamRead)
{
	u8 buffer[] = { 0x11u, 0x22u, 0x33u, 0x44u, 0x55u, 0x66, 0x77 };

	ConstLittleEndianBitStream stream(ConstByteData(buffer, sizeof(buffer)));

	ASSERT_EQ(( (u8)stream.Read< 8>()), 0x11u);
	ASSERT_EQ(((u16)stream.Read<16>()), 0x3322u);
	ASSERT_EQ(((u32)stream.Read<32>()), 0x77665544u);
}


TEST(BitsGetterTest, BigEndianBitStreamWrite)
{
	u8 buffer[sizeof(u8) + sizeof(u16) + sizeof(u32)];

	BitStream stream(ByteData(buffer, sizeof(buffer)));

	stream.Write< 8>(0x11u);
	stream.Write<16>(0x2233u);
	stream.Write<32>(0x44556677u);

	ASSERT_EQ(buffer[0], 0x11u);
	ASSERT_EQ(buffer[1], 0x22u);
	ASSERT_EQ(buffer[2], 0x33u);
	ASSERT_EQ(buffer[3], 0x44u);
	ASSERT_EQ(buffer[4], 0x55u);
	ASSERT_EQ(buffer[5], 0x66u);
	ASSERT_EQ(buffer[6], 0x77u);
}


TEST(BitsGetterTest, LittleEndianBitStreamWrite)
{
	u8 buffer[sizeof(u8) + sizeof(u16) + sizeof(u32)];

	LittleEndianBitStream stream(ByteData(buffer, sizeof(buffer)));

	stream.Write< 8>(0x11u);
	stream.Write<16>(0x2233u);
	stream.Write<32>(0x44556677u);

	ASSERT_EQ(buffer[0], 0x11u);
	ASSERT_EQ(buffer[1], 0x33u);
	ASSERT_EQ(buffer[2], 0x22u);
	ASSERT_EQ(buffer[3], 0x77u);
	ASSERT_EQ(buffer[4], 0x66u);
	ASSERT_EQ(buffer[5], 0x55u);
	ASSERT_EQ(buffer[6], 0x44u);
}
