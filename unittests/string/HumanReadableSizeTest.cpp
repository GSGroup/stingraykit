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
