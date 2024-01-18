// Copyright (c) 2011 - 2024, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/exception.h>

#include <gtest/gtest.h>

using namespace stingray;


#if 0
TEST(ExceptionTest, MustBeUncompilable)
{
	STINGRAYKIT_CHECK_INTEGER_OVERFLOW((s8)0, (u8)0);
	STINGRAYKIT_CHECK_INTEGER_OVERFLOW((s16)0, (u16)0);
	STINGRAYKIT_CHECK_INTEGER_OVERFLOW((s32)0, (u32)0);
	STINGRAYKIT_CHECK_INTEGER_OVERFLOW((s64)0, (u64)0);

	STINGRAYKIT_CHECK_INTEGER_OVERFLOW((u8)0, (u16)0);
	STINGRAYKIT_CHECK_INTEGER_OVERFLOW((u16)0, (u32)0);
	STINGRAYKIT_CHECK_INTEGER_OVERFLOW((u32)0, (u64)0);
}
#endif


TEST(ExceptionTest, IntegerOverflow)
{
	ASSERT_NO_THROW(STINGRAYKIT_CHECK_INTEGER_OVERFLOW(0, 0));
	ASSERT_NO_THROW(STINGRAYKIT_CHECK_INTEGER_OVERFLOW(0, 1));
	ASSERT_NO_THROW(STINGRAYKIT_CHECK_INTEGER_OVERFLOW(0, std::numeric_limits<int>::max()));
	ASSERT_NO_THROW(STINGRAYKIT_CHECK_INTEGER_OVERFLOW(1, std::numeric_limits<int>::max() - 1));

	ASSERT_THROW(STINGRAYKIT_CHECK_INTEGER_OVERFLOW(1, std::numeric_limits<int>::max()), IntegerOverflowException);

	ASSERT_NO_THROW(STINGRAYKIT_CHECK_INTEGER_OVERFLOW(0u, 0u));
	ASSERT_NO_THROW(STINGRAYKIT_CHECK_INTEGER_OVERFLOW(0u, 1u));
	ASSERT_NO_THROW(STINGRAYKIT_CHECK_INTEGER_OVERFLOW(0u, std::numeric_limits<unsigned>::max()));
	ASSERT_NO_THROW(STINGRAYKIT_CHECK_INTEGER_OVERFLOW(1u, std::numeric_limits<unsigned>::max() - 1));

	ASSERT_THROW(STINGRAYKIT_CHECK_INTEGER_OVERFLOW(1u, std::numeric_limits<unsigned>::max()), IntegerOverflowException);

	ASSERT_NO_THROW(STINGRAYKIT_CHECK_INTEGER_OVERFLOW((s8)0, std::numeric_limits<s8>::max()));
	ASSERT_THROW(STINGRAYKIT_CHECK_INTEGER_OVERFLOW((s8)1, std::numeric_limits<s8>::max()), IntegerOverflowException);

	ASSERT_NO_THROW(STINGRAYKIT_CHECK_INTEGER_OVERFLOW((u8)(std::numeric_limits<u8>::max() / 2 + 1), std::numeric_limits<s8>::max()));
	ASSERT_THROW(STINGRAYKIT_CHECK_INTEGER_OVERFLOW((u8)(std::numeric_limits<u8>::max() / 2 + 2), std::numeric_limits<s8>::max()), IntegerOverflowException);

	ASSERT_NO_THROW(STINGRAYKIT_CHECK_INTEGER_OVERFLOW((s16)(std::numeric_limits<s16>::max() - (s16)std::numeric_limits<s8>::max() * 2 - 1), std::numeric_limits<u8>::max()));
	ASSERT_THROW(STINGRAYKIT_CHECK_INTEGER_OVERFLOW((s16)(std::numeric_limits<s16>::max() - (s16)std::numeric_limits<s8>::max() * 2), std::numeric_limits<u8>::max()), IntegerOverflowException);

	ASSERT_NO_THROW(STINGRAYKIT_CHECK_INTEGER_OVERFLOW((s16)0, std::numeric_limits<s16>::max()));
	ASSERT_THROW(STINGRAYKIT_CHECK_INTEGER_OVERFLOW((s16)1, std::numeric_limits<s16>::max()), IntegerOverflowException);

	ASSERT_NO_THROW(STINGRAYKIT_CHECK_INTEGER_OVERFLOW((u16)(std::numeric_limits<u16>::max() / 2 + 1), std::numeric_limits<s16>::max()));
	ASSERT_THROW(STINGRAYKIT_CHECK_INTEGER_OVERFLOW((u16)(std::numeric_limits<u16>::max() / 2 + 2), std::numeric_limits<s16>::max()), IntegerOverflowException);

	ASSERT_NO_THROW(STINGRAYKIT_CHECK_INTEGER_OVERFLOW((s32)(std::numeric_limits<s32>::max() - (s32)std::numeric_limits<s16>::max() * 2 - 1), std::numeric_limits<u16>::max()));
	ASSERT_THROW(STINGRAYKIT_CHECK_INTEGER_OVERFLOW((s32)(std::numeric_limits<s32>::max() - (s32)std::numeric_limits<s16>::max() * 2), std::numeric_limits<u16>::max()), IntegerOverflowException);

	ASSERT_NO_THROW(STINGRAYKIT_CHECK_INTEGER_OVERFLOW((s32)0, std::numeric_limits<s32>::max()));
	ASSERT_THROW(STINGRAYKIT_CHECK_INTEGER_OVERFLOW((s32)1, std::numeric_limits<s32>::max()), IntegerOverflowException);

	ASSERT_NO_THROW(STINGRAYKIT_CHECK_INTEGER_OVERFLOW((u32)(std::numeric_limits<u32>::max() / 2 + 1), std::numeric_limits<s32>::max()));
	ASSERT_THROW(STINGRAYKIT_CHECK_INTEGER_OVERFLOW((u32)(std::numeric_limits<u32>::max() / 2 + 2), std::numeric_limits<s32>::max()), IntegerOverflowException);

	ASSERT_NO_THROW(STINGRAYKIT_CHECK_INTEGER_OVERFLOW((s64)(std::numeric_limits<s64>::max() - (s64)std::numeric_limits<s32>::max() * 2 - 1), std::numeric_limits<u32>::max()));
	ASSERT_THROW(STINGRAYKIT_CHECK_INTEGER_OVERFLOW((s64)(std::numeric_limits<s64>::max() - (s64)std::numeric_limits<s32>::max() * 2), std::numeric_limits<u32>::max()), IntegerOverflowException);

	ASSERT_NO_THROW(STINGRAYKIT_CHECK_INTEGER_OVERFLOW((s64)0, std::numeric_limits<s64>::max()));
	ASSERT_THROW(STINGRAYKIT_CHECK_INTEGER_OVERFLOW((s64)1, std::numeric_limits<s64>::max()), IntegerOverflowException);

	ASSERT_NO_THROW(STINGRAYKIT_CHECK_INTEGER_OVERFLOW((u64)(std::numeric_limits<u64>::max() / 2 + 1), std::numeric_limits<s64>::max()));
	ASSERT_THROW(STINGRAYKIT_CHECK_INTEGER_OVERFLOW((u64)(std::numeric_limits<u64>::max() / 2 + 2), std::numeric_limits<s64>::max()), IntegerOverflowException);
}


TEST(ExceptionTest, IntegerUnderflow)
{
	ASSERT_NO_THROW(STINGRAYKIT_CHECK_INTEGER_OVERFLOW(0, -1));
	ASSERT_NO_THROW(STINGRAYKIT_CHECK_INTEGER_OVERFLOW(0, std::numeric_limits<int>::min()));
	ASSERT_NO_THROW(STINGRAYKIT_CHECK_INTEGER_OVERFLOW(-1, std::numeric_limits<int>::min() + 1));

	ASSERT_THROW(STINGRAYKIT_CHECK_INTEGER_OVERFLOW(-1, std::numeric_limits<int>::min()), IntegerOverflowException);

	ASSERT_NO_THROW(STINGRAYKIT_CHECK_INTEGER_OVERFLOW(1u, -1));
	ASSERT_THROW(STINGRAYKIT_CHECK_INTEGER_OVERFLOW(1u, -2), IntegerOverflowException);

	ASSERT_NO_THROW(STINGRAYKIT_CHECK_INTEGER_OVERFLOW(std::numeric_limits<unsigned>::max() / 2u + 1u, std::numeric_limits<int>::min()));
	ASSERT_THROW(STINGRAYKIT_CHECK_INTEGER_OVERFLOW(std::numeric_limits<unsigned>::max() / 2u, std::numeric_limits<int>::min()), IntegerOverflowException);

	ASSERT_NO_THROW(STINGRAYKIT_CHECK_INTEGER_OVERFLOW((s8)0, std::numeric_limits<s8>::min()));
	ASSERT_THROW(STINGRAYKIT_CHECK_INTEGER_OVERFLOW((s8)-1, std::numeric_limits<s8>::min()), IntegerOverflowException);

	ASSERT_NO_THROW(STINGRAYKIT_CHECK_INTEGER_OVERFLOW((u8)(std::numeric_limits<u8>::max() / 2 + 1), std::numeric_limits<s8>::min()));
	ASSERT_THROW(STINGRAYKIT_CHECK_INTEGER_OVERFLOW((u8)(std::numeric_limits<u8>::max() / 2), std::numeric_limits<s8>::min()), IntegerOverflowException);

	ASSERT_NO_THROW(STINGRAYKIT_CHECK_INTEGER_OVERFLOW((s16)(std::numeric_limits<s16>::min() - std::numeric_limits<s8>::min()), std::numeric_limits<s8>::min()));
	ASSERT_THROW(STINGRAYKIT_CHECK_INTEGER_OVERFLOW((s16)(std::numeric_limits<s16>::min() - std::numeric_limits<s8>::min() - 1), std::numeric_limits<s8>::min()), IntegerOverflowException);

	ASSERT_NO_THROW(STINGRAYKIT_CHECK_INTEGER_OVERFLOW((s16)0, std::numeric_limits<s16>::min()));
	ASSERT_THROW(STINGRAYKIT_CHECK_INTEGER_OVERFLOW((s16)-1, std::numeric_limits<s16>::min()), IntegerOverflowException);

	ASSERT_NO_THROW(STINGRAYKIT_CHECK_INTEGER_OVERFLOW((u16)(std::numeric_limits<u16>::max() / 2 + 1), std::numeric_limits<s16>::min()));
	ASSERT_THROW(STINGRAYKIT_CHECK_INTEGER_OVERFLOW((u16)(std::numeric_limits<u16>::max() / 2), std::numeric_limits<s16>::min()), IntegerOverflowException);

	ASSERT_NO_THROW(STINGRAYKIT_CHECK_INTEGER_OVERFLOW((s32)(std::numeric_limits<s32>::min() - std::numeric_limits<s16>::min()), std::numeric_limits<s16>::min()));
	ASSERT_THROW(STINGRAYKIT_CHECK_INTEGER_OVERFLOW((s32)(std::numeric_limits<s32>::min() - std::numeric_limits<s16>::min() - 1), std::numeric_limits<s16>::min()), IntegerOverflowException);

	ASSERT_NO_THROW(STINGRAYKIT_CHECK_INTEGER_OVERFLOW((s32)0, std::numeric_limits<s32>::min()));
	ASSERT_THROW(STINGRAYKIT_CHECK_INTEGER_OVERFLOW((s32)-1, std::numeric_limits<s32>::min()), IntegerOverflowException);

	ASSERT_NO_THROW(STINGRAYKIT_CHECK_INTEGER_OVERFLOW((u32)(std::numeric_limits<u32>::max() / 2 + 1), std::numeric_limits<s32>::min()));
	ASSERT_THROW(STINGRAYKIT_CHECK_INTEGER_OVERFLOW((u32)(std::numeric_limits<u32>::max() / 2), std::numeric_limits<s32>::min()), IntegerOverflowException);

	ASSERT_NO_THROW(STINGRAYKIT_CHECK_INTEGER_OVERFLOW((s64)(std::numeric_limits<s64>::min() - std::numeric_limits<s32>::min()), std::numeric_limits<s32>::min()));
	ASSERT_THROW(STINGRAYKIT_CHECK_INTEGER_OVERFLOW((s64)(std::numeric_limits<s64>::min() - std::numeric_limits<s32>::min() - 1), std::numeric_limits<s32>::min()), IntegerOverflowException);

	ASSERT_NO_THROW(STINGRAYKIT_CHECK_INTEGER_OVERFLOW((s64)0, std::numeric_limits<s64>::min()));
	ASSERT_THROW(STINGRAYKIT_CHECK_INTEGER_OVERFLOW((s64)-1, std::numeric_limits<s64>::min()), IntegerOverflowException);

	ASSERT_NO_THROW(STINGRAYKIT_CHECK_INTEGER_OVERFLOW((u64)(std::numeric_limits<u64>::max() / 2 + 1), std::numeric_limits<s64>::min()));
	ASSERT_THROW(STINGRAYKIT_CHECK_INTEGER_OVERFLOW((u64)(std::numeric_limits<u64>::max() / 2), std::numeric_limits<s64>::min()), IntegerOverflowException);
}
