#ifndef STINGRAYKIT_METAPROGRAMMING_METHODCHECK_H
#define STINGRAYKIT_METAPROGRAMMING_METHODCHECK_H

// Copyright (c) 2011 - 2024, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/metaprogramming/TypeTraits.h>

namespace stingray
{

#define STINGRAYKIT_DECLARE_METHOD_CHECK(Method_) \
	template < typename T > \
	class HasMethod_##Method_ \
	{ \
		template <typename Type_> \
		class Impl \
		{ \
			struct BaseMixin { void Method_() { } }; \
			struct Base : public Type_, public BaseMixin { Base(); }; \
			\
			template < typename V, V t > class Helper { }; \
			\
			template < typename U > \
			static stingray::FalseType deduce(U*, Helper<void (BaseMixin::*)(), &U::Method_>* = 0); \
			static stingray::TrueType deduce(...); \
			\
		public: \
			static const bool Value = decltype(deduce((Base*)0))::Value; \
		}; \
		\
	public: \
		static const bool Value = \
			stingray::If< \
					stingray::IsClass<T>::Value, \
					Impl<T>, \
					stingray::FalseType \
				>::ValueT::Value; \
	}

}

#endif
