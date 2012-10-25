#ifndef __GS_DVRLIB_TOOLKIT_TYPES_H__
#define __GS_DVRLIB_TOOLKIT_TYPES_H__


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

#if 0 // to avoid 'unsigned < 0' comparison warnings
#define DETAIL_TOOLKIT_DECLARE_INT_TYPE(NativeType_, BitsCount_) \
	typedef unsigned NativeType_	u##BitsCount_; \
	typedef signed NativeType_		s##BitsCount_; \
	TOOLKIT_ASSERT_SIZE_OF(u##BitsCount_,	BitsCount_ / 8) \
	TOOLKIT_ASSERT_SIZE_OF(s##BitsCount_,	BitsCount_ / 8) \
	TOOLKIT_ASSERT_SIGNED(u##BitsCount_,	false) \
	TOOLKIT_ASSERT_SIGNED(s##BitsCount_,	true) \
	template <> struct IntType<BitsCount_, true> { typedef s##BitsCount_ ValueT; }; \
	template <> struct IntType<BitsCount_, false> { typedef u##BitsCount_ ValueT; }
#endif
	
	template < size_t BitsCount, bool Signed = false >
	struct IntType;

#if 0 // to avoid 'unsigned < 0' comparison warnings
	DETAIL_TOOLKIT_DECLARE_INT_TYPE(char,		8);
	DETAIL_TOOLKIT_DECLARE_INT_TYPE(short,		16);
	DETAIL_TOOLKIT_DECLARE_INT_TYPE(int,		32);
	DETAIL_TOOLKIT_DECLARE_INT_TYPE(long long,	64);
#endif

	typedef int8_t		s8;
	typedef uint8_t		u8;
	typedef int16_t		s16;
	typedef uint16_t	u16;
	typedef int32_t		s32;
	typedef uint32_t	u32;
	typedef long long	s64; // to avoid type conflicts with existing 123LL-like constants and "%lld" format specs
	TOOLKIT_ASSERT_SIZE_OF(s64, 8); // de bene esse
	typedef uint64_t	u64;
	template <> struct IntType<8, true> { typedef s8 ValueT; };
	template <> struct IntType<8, false> { typedef u8 ValueT; };
	template <> struct IntType<16, true> { typedef s16 ValueT; };
	template <> struct IntType<16, false> { typedef u16 ValueT; };
	template <> struct IntType<32, true> { typedef s32 ValueT; };
	template <> struct IntType<32, false> { typedef u32 ValueT; };
	template <> struct IntType<64, true> { typedef s64 ValueT; };
	template <> struct IntType<64, false> { typedef u64 ValueT; };

	typedef TYPELIST(unsigned, unsigned long, size_t, int, off_t, u8, s8, u16, s16, u32, s32, u64, s64, float, double, bool) NotClassTypes;
	typedef TYPELIST(unsigned, unsigned long, size_t, int, off_t, u8, s8, u16, s16, u32, s32, u64, s64) IntTypes;
	typedef TYPELIST(u8, s8, u16, s16, u32, s32, u64, s64) FixedWidthIntTypes;

	template < typename T >
	struct IsIntType
	{ static const bool Value = TypeListContains<IntTypes, T>::Value; };

	template < typename T >
	struct IsFixedWidthIntType
	{ static const bool Value = TypeListContains<FixedWidthIntTypes, T>::Value; };

	/*! \endcond */

}


#endif
