// Copyright (c) 2011 - 2024, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/log/LogLevel.h>
#include <stingraykit/shared_ptr.h>

#include <gtest/gtest.h>

using namespace stingray;

namespace
{

	template < typename T >
	struct AlignmentHelper
	{
		template < size_t Count_ >
		struct H
		{ T arr[Count_]; };

		typedef TypeList<H<1>, H<2>, H<3>, H<4>, H<5>, H<6>, H<7>, H<8>, H<9>, H<10> > ValueT;
	};

	typedef TypeList<shared_ptr<int>, unique_ptr<u64>, std::string, LogLevel> AdditionalTypes;
	typedef TypeListMerge<TypeList<BuiltinTypes, AdditionalTypes>>::ValueT BasicTypes;
	typedef TypeListTransform<BasicTypes, AlignmentHelper>::ValueT Transformed;
	typedef TypeListMerge<Transformed>::ValueT AllTypes;


#if defined(__GNUC__)
#	define DETAIL_STINGRAY_ALIGNMENT_CHECK(Type_, Actual_) static_assert(__alignof(Type_) == Actual_, "Wrong alignment")
#	define DETAIL_STINGRAY_VALUE_ALIGNMENT_CHECK(Lhs_, Rhs_) DETAIL_STINGRAY_ALIGNMENT_CHECK(Lhs_, __alignof(Rhs_))
#else
#	define DETAIL_STINGRAY_ALIGNMENT_CHECK(...) (void)0
#	define DETAIL_STINGRAY_VALUE_ALIGNMENT_CHECK(...) (void)0
#	warning "Can't check alignment!"
#endif

	template < typename T >
	struct AlignmentChecker
	{
		static void Call()
		{ DETAIL_STINGRAY_ALIGNMENT_CHECK(T, alignment_of<T>::Value); }
	};

	template < typename T >
	struct AlignedStorageChecker
	{
		static void Call()
		{
			StorageFor<T> storage;
			static_assert(sizeof(T) == sizeof(storage._value), "Wrong size");
			DETAIL_STINGRAY_VALUE_ALIGNMENT_CHECK(T, storage._value);
		}
	};

	template < typename Alignment_ >
	struct TypeWithAlignmentChecker
	{
		static const size_t Align = Alignment_::Value;

		static void Call()
		{
			typedef typename Detail::AlignedTypes::type_with_alignment<Align>::type TWA;
			DETAIL_STINGRAY_ALIGNMENT_CHECK(TWA, Align);
		}
	};

#undef DETAIL_STINGRAY_VALUE_ALIGNMENT_CHECK
#undef DETAIL_STINGRAY_ALIGNMENT_CHECK

}


TEST(AlignedStorageTest, Alignment)
{ ForEachInTypeList<AllTypes, AlignmentChecker>::Do(); }


TEST(AlignedStorageTest, AlignedStorage)
{ ForEachInTypeList<AllTypes, AlignedStorageChecker>::Do(); }


TEST(AlignedStorageTest, TypeWithAlignment)
{ ForEachInTypeList<Detail::AlignedTypes::SupportedAligners, TypeWithAlignmentChecker>::Do(); }
