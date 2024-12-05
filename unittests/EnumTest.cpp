// Copyright (c) 2011 - 2024, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/Enum.h>

#include <gtest/gtest.h>

using namespace stingray;

namespace
{

	struct SampleEnum
	{
		STINGRAYKIT_ENUM_VALUES
		(
			Zero,
			One,
			Two,
			Three,
			Four,
			Five,
			Six,
			Seven,
			Eight,
			Nine
		);
		STINGRAYKIT_DECLARE_ENUM_CLASS(SampleEnum);
	};
	STINGRAYKIT_DECLARE_ENUM_CLASS_BIT_OPERATORS(SampleEnum);

}


TEST(EnumTest, ToString)
{
	ASSERT_EQ(SampleEnum(SampleEnum::Zero).ToString(), "Zero");
	ASSERT_EQ(SampleEnum(SampleEnum::One).ToString(), "One");
	ASSERT_EQ(SampleEnum(SampleEnum::Two).ToString(), "Two");
	ASSERT_EQ(SampleEnum(SampleEnum::Three).ToString(), "Three");
	ASSERT_EQ(SampleEnum(SampleEnum::Four).ToString(), "Four");
	ASSERT_EQ(SampleEnum(SampleEnum::Five).ToString(), "Five");
	ASSERT_EQ(SampleEnum(SampleEnum::Six).ToString(), "Six");
	ASSERT_EQ(SampleEnum(SampleEnum::Seven).ToString(), "Seven");
	ASSERT_EQ(SampleEnum(SampleEnum::Eight).ToString(), "Eight");
	ASSERT_EQ(SampleEnum(SampleEnum::Nine).ToString(), "Nine");
}


TEST(EnumTest, FromString)
{
	ASSERT_TRUE(SampleEnum::FromString("Zero") == SampleEnum::Zero);
	ASSERT_TRUE(SampleEnum::FromString("One") == SampleEnum::One);
	ASSERT_TRUE(SampleEnum::FromString("Two") == SampleEnum::Two);
	ASSERT_TRUE(SampleEnum::FromString("Three") == SampleEnum::Three);
	ASSERT_TRUE(SampleEnum::FromString("Four") == SampleEnum::Four);
	ASSERT_TRUE(SampleEnum::FromString("Five") == SampleEnum::Five);
	ASSERT_TRUE(SampleEnum::FromString("Six") == SampleEnum::Six);
	ASSERT_TRUE(SampleEnum::FromString("Seven") == SampleEnum::Seven);
	ASSERT_TRUE(SampleEnum::FromString("Eight") == SampleEnum::Eight);
	ASSERT_TRUE(SampleEnum::FromString("Nine") == SampleEnum::Nine);
}


TEST(EnumTest, Iterators)
{
	SampleEnum::const_iterator seit = SampleEnum::begin();
	ASSERT_TRUE(*seit++ == SampleEnum::Zero);
	ASSERT_TRUE(*seit++ == SampleEnum::One);
	ASSERT_TRUE(*seit++ == SampleEnum::Two);
	ASSERT_TRUE(*seit++ == SampleEnum::Three);
	ASSERT_TRUE(*seit++ == SampleEnum::Four);
	ASSERT_TRUE(*seit++ == SampleEnum::Five);
	ASSERT_TRUE(*seit++ == SampleEnum::Six);
	ASSERT_TRUE(*seit++ == SampleEnum::Seven);
	ASSERT_TRUE(*seit++ == SampleEnum::Eight);
	ASSERT_TRUE(*seit++ == SampleEnum::Nine);
	ASSERT_TRUE(seit == SampleEnum::end());
}


TEST(EnumTest, ComparisonOps)
{
	for (SampleEnum::const_iterator it = SampleEnum::begin(); it != SampleEnum::end(); ++it)
	{
		const SampleEnum other = *it;
		ASSERT_EQ(*it, other);
	}

	for (SampleEnum::const_iterator prev = SampleEnum::begin(), cur = std::next(SampleEnum::begin()); cur != SampleEnum::end(); ++cur, ++prev)
		ASSERT_NE(*prev, *cur);

	for (SampleEnum::const_iterator prev = SampleEnum::begin(), cur = std::next(SampleEnum::begin()); cur != SampleEnum::end(); ++cur, ++prev)
		ASSERT_LT(*prev, *cur);

	for (SampleEnum::const_iterator prev = SampleEnum::begin(), cur = SampleEnum::begin(); cur != SampleEnum::end(); ++cur, ++prev)
		ASSERT_LE(*prev, *cur);

	for (SampleEnum::const_iterator prev = SampleEnum::begin(), cur = std::next(SampleEnum::begin()); cur != SampleEnum::end(); ++cur, ++prev)
		ASSERT_GT(*cur, *prev);

	for (SampleEnum::const_iterator prev = SampleEnum::begin(), cur = SampleEnum::begin(); cur != SampleEnum::end(); ++cur, ++prev)
		ASSERT_GE(*cur, *prev);
}


TEST(EnumTest, BitwiseOps)
{
	ASSERT_TRUE((SampleEnum(SampleEnum::One) | SampleEnum(SampleEnum::Two)) == SampleEnum(SampleEnum::Three));
	ASSERT_TRUE((SampleEnum(SampleEnum::One) & SampleEnum(SampleEnum::Two)) == SampleEnum(SampleEnum::Zero));

	SampleEnum se = SampleEnum::Zero;
	se |= SampleEnum::One;
	ASSERT_TRUE(se == SampleEnum::One);
	se |= SampleEnum::Two;
	ASSERT_TRUE(se == SampleEnum::Three);
	se &= SampleEnum::Two;
	ASSERT_TRUE(se == SampleEnum::Two);
}
