#ifndef STINGRAYKIT_METAPROGRAMMING_TYPERELATIONSHIPS_H
#define STINGRAYKIT_METAPROGRAMMING_TYPERELATIONSHIPS_H

// Copyright (c) 2011 - 2025, GS Group, https://github.com/GSGroup
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

		template < template <typename...> class Base, typename... Ts >	TrueType	TestIsInheritedFromTemplate(const Base<Ts...>*);
		template < template <typename...> class Base >					FalseType	TestIsInheritedFromTemplate(...);

		template < typename From, typename To > auto		TestIsConvertible(int) -> decltype(std::declval<void (&) (To)>()(std::declval<From>()), TrueType());
		template < typename From, typename To > FalseType	TestIsConvertible(...);

		template < typename To, typename... Args > auto        TestIsConstructible(int) -> decltype(To(std::declval<Args>()...), TrueType());
		template < typename To, typename... Args > FalseType	TestIsConstructible(...);

		template < typename To, typename From > auto        TestIsAssignable(int) -> decltype(std::declval<To>() = std::declval<From>(), TrueType());
		template < typename To, typename From > FalseType	TestIsAssignable(...);

	}

	template < typename Derived, typename Base > struct IsInherited : If<IsSame<Derived, Base>::Value, integral_constant<bool, true>, Detail::IsInheritedImpl<Derived, Base> >::ValueT { };

	template < typename Derived, template <typename...> class Base > struct IsInheritedFromTemplate : integral_constant<bool, decltype(Detail::TestIsInheritedFromTemplate<Base>((const Derived*)0))::Value> { };


	template < typename From, typename To > struct IsConvertible : integral_constant<bool, decltype(Detail::TestIsConvertible<From, To>(0))::Value> { };

	template < typename To, typename... Args > struct IsConstructible : integral_constant<bool, decltype(Detail::TestIsConstructible<To, Args...>(0))::Value> { };
	template < typename To > struct IsDefaultConstructible : IsConstructible<To> { };

	template < typename To, typename From > struct IsAssignable : integral_constant<bool, decltype(Detail::TestIsAssignable<To, From>(0))::Value> { };


	template < typename T > struct IsMoveConstructible : integral_constant<bool, IsConstructible<T, T>::Value && IsConvertible<T, T>::Value> { };
	template < typename T > struct IsCopyConstructible : integral_constant<bool,
			IsMoveConstructible<T>::Value &&
			IsConstructible<T, T&>::Value && IsConstructible<T, const T&>::Value && IsConstructible<T, const T>::Value &&
			IsConvertible<T&, T>::Value && IsConvertible<const T&, T>::Value && IsConvertible<const T, T>::Value>
	{ };

}

#endif
