#ifndef STINGRAYKIT_METAPROGRAMMING_TYPERELATIONSHIPS_H
#define STINGRAYKIT_METAPROGRAMMING_TYPERELATIONSHIPS_H

// Copyright (c) 2011 - 2017, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/metaprogramming/IntegralConstant.h>
#include <stingraykit/metaprogramming/TypeTraits.h>
#include <stingraykit/metaprogramming/YesNo.h>

namespace stingray
{

	template <typename T, typename U> struct IsSame { static const bool Value = false; };
	template <typename T> struct IsSame<T, T> { static const bool Value = true; };

	namespace Detail
	{
		template <typename Derived, typename Base>
		class IsInheritedImpl
		{
		private:
			static YesType TestInheritance(const Base*);
			static NoType TestInheritance(...);

		public:
			static const bool Value = ( sizeof(TestInheritance((const Derived*)0)) == sizeof(YesType) ) && IsComplete<Derived>::Value;
		};
	}

	template <typename Derived, typename Base>
	struct IsInherited : If<IsSame<Derived, Base>::Value, integral_constant<bool, true>, Detail::IsInheritedImpl<Derived, Base> >::ValueT { };

	template < typename Derived, template <typename> class Base>
	class IsInherited1ParamTemplate
	{
	private:
		template < typename T >
		static YesType TestInheritance(const Base<T>*);
		static NoType TestInheritance(...);

	public:
		static const bool Value = ( sizeof(TestInheritance((const Derived*)0)) == sizeof(YesType) );
	};

	template < typename Derived, template <typename, typename> class Base>
	class IsInherited2ParamTemplate
	{
	private:
		template < typename T1, typename T2 >
		static YesType TestInheritance(const Base<T1, T2>*);
		static NoType TestInheritance(...);

	public:
		static const bool Value = ( sizeof(TestInheritance((const Derived*)0)) == sizeof(YesType) );
	};


	template < typename From, typename To>
	struct IsConvertible
	{
		static YesType Test(To);
		static NoType Test(...);

		static const bool Value = sizeof(Test(*((const From*)0))) == sizeof(YesType);
	};


	template < template <typename> class Template, typename U >
	struct Is1ParamTemplate { static const bool Value = false; };
	template < template <typename> class Template, typename T >
	struct Is1ParamTemplate<Template, Template<T> > { static const bool Value = true; };

	template < template <typename, typename> class Template, typename U >
	struct Is2ParamTemplate { static const bool Value = false; };
	template < template <typename, typename> class Template, typename T, typename U >
	struct Is2ParamTemplate<Template, Template<T, U> > { static const bool Value = true; };

}

#endif
