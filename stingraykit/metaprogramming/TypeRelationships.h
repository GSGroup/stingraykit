#ifndef STINGRAYKIT_METAPROGRAMMING_TYPERELATIONSHIPS_H
#define STINGRAYKIT_METAPROGRAMMING_TYPERELATIONSHIPS_H

// Copyright (c) 2011 - 2019, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/metaprogramming/If.h>
#include <stingraykit/metaprogramming/IntegralConstant.h>
#include <stingraykit/metaprogramming/TypeCompleteness.h>
#include <stingraykit/metaprogramming/YesNo.h>

namespace stingray
{

	template <typename T, typename U> struct IsSame	: FalseType	{ };
	template <typename T> struct IsSame<T, T>		: TrueType	{ };

	namespace Detail
	{

		template < typename Base > YesType	TestIsInherited(const Base*);
		template < typename Base > NoType	TestIsInherited(...);

		template < typename Derived, typename Base > struct IsInheritedImpl : integral_constant<bool, ( sizeof(TestIsInherited<Base>((const typename StaticAssertCompleteType<Derived>::ValueT*)0)) == sizeof(YesType) )> { };


		template < template <typename> class Base, typename T >	YesType	TestIsInherited1ParamTemplate(const Base<T>*);
		template < template <typename> class Base>				NoType	TestIsInherited1ParamTemplate(...);


		template < template <typename, typename> class Base, typename T1, typename T2 >	YesType	TestIsInherited2ParamTemplate(const Base<T1, T2>*);
		template < template <typename, typename> class Base >							NoType	TestIsInherited2ParamTemplate(...);


		template < typename T > YesType	TestIsConvertible(T);
		template < typename T > NoType	TestIsConvertible(...);

	}

	template < typename Derived, typename Base > struct IsInherited : If<IsSame<Derived, Base>::Value, integral_constant<bool, true>, Detail::IsInheritedImpl<Derived, Base> >::ValueT { };


	template < typename Derived, template <typename > class Base>			struct IsInherited1ParamTemplate : integral_constant<bool, sizeof(Detail::TestIsInherited1ParamTemplate<Base>((const Derived*)0)) == sizeof(YesType)> { };
	template < typename Derived, template <typename, typename > class Base>	struct IsInherited2ParamTemplate : integral_constant<bool, sizeof(Detail::TestIsInherited2ParamTemplate<Base>((const Derived*)0)) == sizeof(YesType)> { };


	template < typename From, typename To > struct IsConvertible : integral_constant<bool, sizeof(Detail::TestIsConvertible<To>(*((const From*)0))) == sizeof(YesType)> { };


	template < template <typename> class Template, typename U > struct Is1ParamTemplate							: FalseType { };
	template < template <typename> class Template, typename T > struct Is1ParamTemplate<Template, Template<T> >	: TrueType { };

	template < template <typename, typename> class Template, typename U > struct Is2ParamTemplate											: FalseType { };
	template < template <typename, typename> class Template, typename T, typename U > struct Is2ParamTemplate<Template, Template<T, U> >	: TrueType { };

}

#endif
