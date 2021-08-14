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
