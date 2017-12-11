#ifndef STINGRAYKIT_METAPROGRAMMING_NESTEDTYPECHECK_H
#define STINGRAYKIT_METAPROGRAMMING_NESTEDTYPECHECK_H

// Copyright (c) 2011 - 2017, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/metaprogramming/If.h>
#include <stingraykit/metaprogramming/IntToType.h>
#include <stingraykit/metaprogramming/TypeTraits.h>
#include <stingraykit/metaprogramming/TypeTransformations.h>
#include <stingraykit/metaprogramming/YesNo.h>

namespace stingray
{

#define STINGRAYKIT_DECLARE_NESTED_TYPE_CHECK(NestedType_) \
	template < typename T > \
	class HasNestedType_##NestedType_ \
	{ \
		template < typename U > static YesType deduce(IntToType<sizeof(typename RemoveReference<typename U::NestedType_>::ValueT*)>*); \
		template < typename U > static NoType deduce(...); \
		\
	public: \
		static const bool Value = sizeof(deduce<T>(0)) == sizeof(YesType); \
	}

	namespace Detail
	{
		struct DoesNotHaveAnyNestedTypes
		{ static const bool Value = false; };
	}

#define STINGRAYKIT_DECLARE_METHOD_CHECK(Method_) \
	template < typename T > \
	struct HasMethod_##Method_ \
	{ \
		template <typename Type_> \
		class Impl \
		{ \
			struct BaseMixin { void Method_(){} }; \
			struct Base : public Type_, public BaseMixin { Base(); }; \
			\
			template <typename V, V t>    class Helper{}; \
			\
			template <typename U> static NoType deduce(U*, Helper<void (BaseMixin::*)(), &U::Method_>* = 0); \
			static YesType deduce(...); \
			\
		public: \
			static const bool Value = (sizeof(YesType) == sizeof(deduce((Base*)(0)))); \
		}; \
		static const bool Value = \
			If	< \
					IsClass<T>::Value, \
					Impl<T>, \
					stingray::Detail::DoesNotHaveAnyNestedTypes \
				>::ValueT::Value; \
	}

}


#endif
