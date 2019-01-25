#ifndef STINGRAYKIT_TYPES_H
#define STINGRAYKIT_TYPES_H

// Copyright (c) 2011 - 2019, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.


#include <sys/types.h>

#include <stingraykit/metaprogramming/Math.h>


namespace stingray
{


#define STINGRAYKIT_ASSERT_SIGNED(Type, Signed) \
		struct Type##SignedChecker \
		{ \
			static const Type Neg = ~(Type)0; \
			static const Type Zero = 0; \
			CompileTimeAssert< (Neg < Zero) == Signed > ERROR__invalid_signed_##Type; \
		};

#define STINGRAYKIT_ASSERT_SIZE_OF(Type, Size) \
		struct Type##SizeChecker\
		{ \
			CompileTimeAssert< sizeof(Type) == Size > ERROR__invalid_size_of_##Type; \
		};

#define DETAIL_STINGRAYKIT_DECLARE_INT_TRAITS(NativeType_) \
	template <> struct IntTraits<signed NativeType_> \
	{ \
		static const size_t BitsCount = sizeof(signed NativeType_) * 8; \
		static const bool IsSigned = true; \
		static const signed NativeType_ Min = (signed NativeType_)(-CompileTimeExponent<unsigned NativeType_, 2, BitsCount - 1>::Value); \
		static const signed NativeType_ Max = (signed NativeType_)( CompileTimeExponent<unsigned NativeType_, 2, BitsCount - 1>::Value - 1); \
	}; \
	template <> struct IntTraits<unsigned NativeType_> \
	{ \
		static const size_t BitsCount = sizeof(unsigned NativeType_) * 8; \
		static const bool IsSigned = false; \
		static const unsigned NativeType_ Min = 0; \
		static const unsigned NativeType_ Max = ~(unsigned NativeType_)0; \
	};

#define DETAIL_STINGRAYKIT_DECLARE_INT_TYPE(NativeType_, BitsCount_) \
	typedef unsigned NativeType_	u##BitsCount_; \
	typedef signed NativeType_		s##BitsCount_; \
	STINGRAYKIT_ASSERT_SIZE_OF(u##BitsCount_,	BitsCount_ / 8) \
	STINGRAYKIT_ASSERT_SIZE_OF(s##BitsCount_,	BitsCount_ / 8) \
	STINGRAYKIT_ASSERT_SIGNED(u##BitsCount_,	false) \
	STINGRAYKIT_ASSERT_SIGNED(s##BitsCount_,	true) \
	template <> struct IntType<BitsCount_, true> \
	{ \
		typedef s##BitsCount_ ValueT; \
		static const ValueT Min = (ValueT)(-CompileTimeExponent<u##BitsCount_, 2, BitsCount_ - 1>::Value); \
		static const ValueT Max = (ValueT)( CompileTimeExponent<u##BitsCount_, 2, BitsCount_ - 1>::Value - 1); \
	}; \
	template <> struct IntType<BitsCount_, false> \
	{ \
		typedef u##BitsCount_ ValueT; \
		static const ValueT Min = 0; \
		static const ValueT Max = ~(u##BitsCount_)0; \
	}; \
	template <> struct IntTraits<u##BitsCount_> \
	{ \
		static const size_t BitsCount = BitsCount_; \
		static const u##BitsCount_ Min = IntType<BitsCount_, false>::Min; \
		static const u##BitsCount_ Max = IntType<BitsCount_, false>::Max; \
	}; \
	template <> struct IntTraits<s##BitsCount_> \
	{ \
		static const size_t BitsCount = BitsCount_; \
		static const s##BitsCount_ Min = IntType<BitsCount_, true>::Min; \
		static const s##BitsCount_ Max = IntType<BitsCount_, true>::Max; \
	};

	template < size_t BitsCount, bool Signed = false >
	struct IntType;

	template < typename IntType_ >
	struct IntTraits;

	DETAIL_STINGRAYKIT_DECLARE_INT_TYPE(char,		8);
	DETAIL_STINGRAYKIT_DECLARE_INT_TYPE(short,		16);
	DETAIL_STINGRAYKIT_DECLARE_INT_TYPE(int,		32);
	DETAIL_STINGRAYKIT_DECLARE_INT_TYPE(long long,	64);

	DETAIL_STINGRAYKIT_DECLARE_INT_TRAITS(long int);

#undef DETAIL_STINGRAYKIT_DECLARE_INT_TRAITS
#undef DETAIL_STINGRAYKIT_DECLARE_INT_TYPE
#undef STINGRAYKIT_ASSERT_SIGNED
#undef STINGRAYKIT_ASSERT_SIZE_OF

}


#endif
