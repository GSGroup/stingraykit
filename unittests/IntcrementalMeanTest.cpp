// Copyright (c) 2011 - 2025, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/IncrementalMean.h>

#include <gmock/gmock-matchers.h>

using ::testing::Property;

using namespace stingray;


#define ASSERT_INCREMENTAL_MEAN(IncrementalMean, MeanValue, RemainderValue, CountValue) \
		ASSERT_THAT(IncrementalMean, AllOf( \
				Property(&decltype(IncrementalMean)::Mean, MeanValue), \
				Property(&decltype(IncrementalMean)::Remainder, RemainderValue), \
				Property(&decltype(IncrementalMean)::Count, CountValue)))


#define CHECK_INCREMENTAL_MEAN(IncrementalMean, Increment, MeanValue, RemainderValue, CountValue) \
		IncrementalMean.Add(Increment); \
		ASSERT_INCREMENTAL_MEAN(IncrementalMean, MeanValue, RemainderValue, CountValue)


TEST(IncrementalMeanTest, Positive)
{
	IncrementalMean<int> mean;
	ASSERT_INCREMENTAL_MEAN(mean, 0, 0, 0);

	CHECK_INCREMENTAL_MEAN(mean, 10, 10, 0, 1);
	CHECK_INCREMENTAL_MEAN(mean, 1, 5, 1, 2);
	CHECK_INCREMENTAL_MEAN(mean, 3, 4, 2, 3);
	CHECK_INCREMENTAL_MEAN(mean, 2, 4, 0, 4);
	CHECK_INCREMENTAL_MEAN(mean, 1, 3, 2, 5);
	CHECK_INCREMENTAL_MEAN(mean, 2, 3, 1, 6);
	CHECK_INCREMENTAL_MEAN(mean, 3, 3, 1, 7);
	CHECK_INCREMENTAL_MEAN(mean, 4, 3, 2, 8);
}


TEST(IncrementalMeanTest, Negative)
{
	IncrementalMean<int> mean;
	ASSERT_INCREMENTAL_MEAN(mean, 0, 0, 0);

	CHECK_INCREMENTAL_MEAN(mean, -10, -10, 0, 1);
	CHECK_INCREMENTAL_MEAN(mean, -1, -6, 1, 2);
	CHECK_INCREMENTAL_MEAN(mean, -3, -5, 1, 3);
	CHECK_INCREMENTAL_MEAN(mean, -2, -4, 0, 4);
	CHECK_INCREMENTAL_MEAN(mean, -1, -4, 3, 5);
	CHECK_INCREMENTAL_MEAN(mean, -2, -4, 5, 6);
	CHECK_INCREMENTAL_MEAN(mean, -3, -4, 6, 7);
	CHECK_INCREMENTAL_MEAN(mean, -4, -4, 6, 8);
}


TEST(IncrementalMeanTest, Mixed)
{
	IncrementalMean<int> mean;
	ASSERT_INCREMENTAL_MEAN(mean, 0, 0, 0);

	CHECK_INCREMENTAL_MEAN(mean, -5, -5, 0, 1);
	CHECK_INCREMENTAL_MEAN(mean, 1, -2, 0, 2);
	CHECK_INCREMENTAL_MEAN(mean, -3, -3, 2, 3);
	CHECK_INCREMENTAL_MEAN(mean, 12, 1, 1, 4);
	CHECK_INCREMENTAL_MEAN(mean, -5, 0, 0, 5);
}


TEST(IncrementalMeanTest, BigNumbers)
{
	{
		IncrementalMean<s64> mean;
		ASSERT_INCREMENTAL_MEAN(mean, 0, 0, 0);

		CHECK_INCREMENTAL_MEAN(mean, 9223372036854775807, 9223372036854775807, 0, 1);
		CHECK_INCREMENTAL_MEAN(mean, 9223372036854775806, 9223372036854775806, 1, 2);
	}

	{
		IncrementalMean<s64> mean;
		ASSERT_INCREMENTAL_MEAN(mean, 0, 0, 0);

		CHECK_INCREMENTAL_MEAN(mean, -9223372036854775807, -9223372036854775807, 0, 1);
		CHECK_INCREMENTAL_MEAN(mean, -9223372036854775806, -9223372036854775807, 1, 2);
	}
}
