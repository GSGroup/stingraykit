#ifndef STINGRAYKIT_METAPROGRAMMING_COMMONTYPE_H
#define STINGRAYKIT_METAPROGRAMMING_COMMONTYPE_H

// Copyright (c) 2011 - 2018, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/metaprogramming/TypeTraits.h>

namespace stingray
{

	namespace Detail
	{

		template < int index > struct BuiltinType;

		template <> struct BuiltinType<1>			{ typedef char (&ResultT)[1];	typedef bool			ValueT; };
		template <> struct BuiltinType<2>			{ typedef char (&ResultT)[2];	typedef char			ValueT; };
		template <> struct BuiltinType<3>			{ typedef char (&ResultT)[3];	typedef wchar_t			ValueT; };

		template <> struct BuiltinType<4>			{ typedef char (&ResultT)[4];	typedef s8				ValueT; };
		template <> struct BuiltinType<5>			{ typedef char (&ResultT)[5];	typedef s16				ValueT; };
		template <> struct BuiltinType<6>			{ typedef char (&ResultT)[6];	typedef s32				ValueT; };
		template <> struct BuiltinType<7>			{ typedef char (&ResultT)[7];	typedef long			ValueT; };
		template <> struct BuiltinType<8>			{ typedef char (&ResultT)[8];	typedef s64				ValueT; };

		template <> struct BuiltinType<9>			{ typedef char (&ResultT)[9];	typedef u8				ValueT; };
		template <> struct BuiltinType<10>			{ typedef char (&ResultT)[10];	typedef u16				ValueT; };
		template <> struct BuiltinType<11>			{ typedef char (&ResultT)[11];	typedef u32				ValueT; };
		template <> struct BuiltinType<12>			{ typedef char (&ResultT)[12];	typedef unsigned long	ValueT; };
		template <> struct BuiltinType<13>			{ typedef char (&ResultT)[13];	typedef u64				ValueT; };

		template <> struct BuiltinType<14>			{ typedef char (&ResultT)[14];	typedef float			ValueT; };
		template <> struct BuiltinType<15>			{ typedef char (&ResultT)[15];	typedef double			ValueT; };
		template <> struct BuiltinType<16>			{ typedef char (&ResultT)[16];	typedef long double		ValueT; };

		template < typename T1, typename T2 >
		class CommonBuiltinType
		{
			static BuiltinType<1>::ResultT	Test(BuiltinType<1>::ValueT);
			static BuiltinType<2>::ResultT	Test(BuiltinType<2>::ValueT);
			static BuiltinType<3>::ResultT	Test(BuiltinType<3>::ValueT);
			static BuiltinType<4>::ResultT	Test(BuiltinType<4>::ValueT);
			static BuiltinType<5>::ResultT	Test(BuiltinType<5>::ValueT);
			static BuiltinType<6>::ResultT	Test(BuiltinType<6>::ValueT);
			static BuiltinType<7>::ResultT	Test(BuiltinType<7>::ValueT);
			static BuiltinType<8>::ResultT	Test(BuiltinType<8>::ValueT);
			static BuiltinType<9>::ResultT	Test(BuiltinType<9>::ValueT);
			static BuiltinType<10>::ResultT	Test(BuiltinType<10>::ValueT);
			static BuiltinType<11>::ResultT	Test(BuiltinType<11>::ValueT);
			static BuiltinType<12>::ResultT	Test(BuiltinType<12>::ValueT);
			static BuiltinType<13>::ResultT	Test(BuiltinType<13>::ValueT);
			static BuiltinType<14>::ResultT	Test(BuiltinType<14>::ValueT);
			static BuiltinType<15>::ResultT	Test(BuiltinType<15>::ValueT);
			static BuiltinType<16>::ResultT	Test(BuiltinType<16>::ValueT);

		public:
			typedef typename BuiltinType<sizeof(Test(bool() ? T1() : T2()))>::ValueT	ValueT;
		};


		// TODO: Add support of pointers
		// TODO: Add support of classes

		struct NoCommonType													{ typedef void ValueT; };

		template < typename T1, typename T2 > struct CommonTypeImpl			: If<IsBuiltin<T1>::Value && IsBuiltin<T2>::Value, CommonBuiltinType<T1, T2>, NoCommonType>::ValueT { };
		template < typename T > struct CommonTypeImpl<T, T>					{ typedef T ValueT; };

	}
	template < typename T1, typename T2 > struct CommonType					: Detail::CommonTypeImpl<typename Decay<T1>::ValueT, typename Decay<T2>::ValueT> { };

}

#endif
