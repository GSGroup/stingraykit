#ifndef STINGRAYKIT_METAPROGRAMMING_TYPERELATIONSHIPS_H
#define STINGRAYKIT_METAPROGRAMMING_TYPERELATIONSHIPS_H

// Copyright (c) 2011 - 2024, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/metaprogramming/If.h>
#include <stingraykit/metaprogramming/IntegralConstant.h>
#include <stingraykit/metaprogramming/TypeCompleteness.h>

#include <type_traits>

namespace stingray
{

	template <typename T, typename U> struct IsSame	: FalseType	{ };
	template <typename T> struct IsSame<T, T>		: TrueType	{ };

	namespace Detail
	{

		template < typename Base > TrueType		TestIsInherited(const Base*);
		template < typename Base > FalseType	TestIsInherited(...);

		template < typename Derived, typename Base > struct IsInheritedImpl : integral_constant<bool, decltype(TestIsInherited<Base>((const typename StaticAssertCompleteType<Derived>::ValueT*)0))::Value> { };


		template < template <typename> class Base, typename T >	TrueType	TestIsInherited1ParamTemplate(const Base<T>*);
		template < template <typename> class Base>				FalseType	TestIsInherited1ParamTemplate(...);


		template < template <typename, typename> class Base, typename T1, typename T2 >	TrueType	TestIsInherited2ParamTemplate(const Base<T1, T2>*);
		template < template <typename, typename> class Base >							FalseType	TestIsInherited2ParamTemplate(...);


		template < typename From, typename To > auto		TestIsConvertible(int) -> decltype(std::declval<void (&) (To)>()(std::declval<From>()), TrueType());
		template < typename From, typename To > FalseType	TestIsConvertible(...);


		template < typename To, typename... Args > auto        TestIsConstructible(int) -> decltype(To(std::declval<Args>()...), TrueType());
		template < typename To, typename... Args > FalseType	TestIsConstructible(...);

		template < typename To, typename From > auto        TestIsAssignable(int) -> decltype(std::declval<To>() = std::declval<From>(), TrueType());
		template < typename To, typename From > FalseType	TestIsAssignable(...);

	}

	template < typename Derived, typename Base > struct IsInherited : If<IsSame<Derived, Base>::Value, integral_constant<bool, true>, Detail::IsInheritedImpl<Derived, Base> >::ValueT { };


	template < typename Derived, template <typename > class Base>			struct IsInherited1ParamTemplate : integral_constant<bool, decltype(Detail::TestIsInherited1ParamTemplate<Base>((const Derived*)0))::Value> { };
	template < typename Derived, template <typename, typename > class Base>	struct IsInherited2ParamTemplate : integral_constant<bool, decltype(Detail::TestIsInherited2ParamTemplate<Base>((const Derived*)0))::Value> { };


	template < typename From, typename To > struct IsConvertible : integral_constant<bool, decltype(Detail::TestIsConvertible<From, To>(0))::Value> { };

	template < typename To, typename... Args > struct IsConstructible : integral_constant<bool, decltype(Detail::TestIsConstructible<To, Args...>(0))::Value> { };
	template < typename To, typename From > struct IsAssignable : integral_constant<bool, decltype(Detail::TestIsAssignable<To, From>(0))::Value> { };


	template < template <typename> class Template, typename U > struct Is1ParamTemplate							: FalseType { };
	template < template <typename> class Template, typename T > struct Is1ParamTemplate<Template, Template<T> >	: TrueType { };

	template < template <typename, typename> class Template, typename U > struct Is2ParamTemplate											: FalseType { };
	template < template <typename, typename> class Template, typename T, typename U > struct Is2ParamTemplate<Template, Template<T, U> >	: TrueType { };

}

#endif
