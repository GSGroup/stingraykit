#ifndef STINGRAY_TOOLKIT_TYPES_H
#define STINGRAY_TOOLKIT_TYPES_H


#include <stdint.h>

#include <stingray/toolkit/TypeList.h>


namespace stingray
{

	/*! \cond GS_INTERNAL */

#define TOOLKIT_ASSERT_SIGNED(Type, Signed) \
		struct Type##SignedChecker \
		{ \
			static const Type Neg = ~(Type)0; \
			static const Type Zero = 0; \
			CompileTimeAssert< (Neg < Zero) == Signed > ERROR__invalid_signed_##Type; \
		};

#define TOOLKIT_ASSERT_SIZE_OF(Type, Size) \
		struct Type##SizeChecker\
		{ \
			CompileTimeAssert< sizeof(Type) == Size > ERROR__invalid_size_of_##Type; \
		};

#define DETAIL_TOOLKIT_DECLARE_INT_TYPE(NativeType_, BitsCount_) \
	typedef unsigned NativeType_	u##BitsCount_; \
	typedef signed NativeType_		s##BitsCount_; \
	TOOLKIT_ASSERT_SIZE_OF(u##BitsCount_,	BitsCount_ / 8) \
	TOOLKIT_ASSERT_SIZE_OF(s##BitsCount_,	BitsCount_ / 8) \
	TOOLKIT_ASSERT_SIGNED(u##BitsCount_,	false) \
	TOOLKIT_ASSERT_SIGNED(s##BitsCount_,	true) \
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
		static const u##BitsCount_ Min = IntType<BitsCount_, false>::Min; \
		static const u##BitsCount_ Max = IntType<BitsCount_, false>::Max; \
	}; \
	template <> struct IntTraits<s##BitsCount_> \
	{ \
		static const s##BitsCount_ Min = IntType<BitsCount_, true>::Min; \
		static const s##BitsCount_ Max = IntType<BitsCount_, true>::Max; \
	};

	template < size_t BitsCount, bool Signed = false >
	struct IntType;

	template < typename IntType_ >
	struct IntTraits;

	DETAIL_TOOLKIT_DECLARE_INT_TYPE(char,		8);
	DETAIL_TOOLKIT_DECLARE_INT_TYPE(short,		16);
	DETAIL_TOOLKIT_DECLARE_INT_TYPE(int,		32);
	DETAIL_TOOLKIT_DECLARE_INT_TYPE(long long,	64);

	typedef TYPELIST(u8, s8, u16, s16, u32, s32, u64, s64) FixedWidthIntTypes;
	typedef TypeListMerge<TypeList_2<FixedWidthIntTypes, TYPELIST(unsigned, unsigned long, size_t, int, long, off_t)> >::ValueT IntTypes;
	typedef TypeListMerge<TypeList_2<IntTypes, TYPELIST(float, double, bool)> >::ValueT BuiltinTypes;

	template < typename T >
	struct IsSigned
	{ static const bool Value = (~(T)0) < ((T)0); };

	template < typename T >
	struct IsIntType
	{ static const bool Value = TypeListContains<IntTypes, T>::Value; };

	template < typename T >
	struct IsFixedWidthIntType
	{ static const bool Value = TypeListContains<FixedWidthIntTypes, T>::Value; };

	template < typename T >
	struct IsBuiltinType
	{ static const bool Value = TypeListContains<BuiltinTypes, T>::Value; };
	/*! \endcond */

}


#endif
