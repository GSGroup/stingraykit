#include <stingraykit/string/Unicode.h>

#include <gtest/gtest.h>

using namespace stingray;

TEST(CollatorTest, CaseSensitiveCompare)
{
	const UnicodeCollator collator(true);

	ASSERT_TRUE(collator.Compare("Ж", "ж") != 0);
	ASSERT_TRUE(collator.Compare("и", "й") < 0);
	ASSERT_TRUE(collator.Compare("Ё", "ё") != 0);
	ASSERT_TRUE(collator.Compare("е", "ё") < 0);
	ASSERT_TRUE(collator.Compare("ё", "ж") < 0);
	ASSERT_TRUE(collator.Compare("Е", "Ё") < 0);
	ASSERT_TRUE(collator.Compare("Ё", "Ж") < 0);
	ASSERT_TRUE(collator.Compare("ёжик", "ёжик земноводный") < 0);
	ASSERT_TRUE(collator.Compare("е", "ёж") < 0);
	ASSERT_TRUE(collator.Compare("ель", "ё") < 0);
	ASSERT_TRUE(collator.Compare("ещё", "ёж") < 0);
	ASSERT_TRUE(collator.Compare("еж", "ёж") < 0);
}


TEST(CollatorTest, CaseInsensitiveCompare)
{
	const UnicodeCollator collator(false);

	ASSERT_TRUE(collator.Compare("Ж", "ж") == 0);
	ASSERT_TRUE(collator.Compare("и", "й") < 0);
	ASSERT_TRUE(collator.Compare("Ё", "ё") == 0);
	ASSERT_TRUE(collator.Compare("е", "ё") < 0);
	ASSERT_TRUE(collator.Compare("ё", "ж") < 0);
	ASSERT_TRUE(collator.Compare("Е", "Ё") < 0);
	ASSERT_TRUE(collator.Compare("Ё", "Ж") < 0);
	ASSERT_TRUE(collator.Compare("ёжик", "ёжик земноводный") < 0);
	ASSERT_TRUE(collator.Compare("е", "ёж") < 0);
	ASSERT_TRUE(collator.Compare("ель", "ё") < 0);
	ASSERT_TRUE(collator.Compare("ещё", "ёж") < 0);
	ASSERT_TRUE(collator.Compare("еж", "ёж") < 0);
}
