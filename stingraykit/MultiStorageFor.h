#ifndef STINGRAYKIT_MULTISTORAGEFOR_H
#define STINGRAYKIT_MULTISTORAGEFOR_H

// Copyright (c) 2011 - 2018, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.


#include <stingraykit/aligned_storage.h>

namespace stingray
{

	template<typename TypeList>
	struct MultiStorageFor
	{
		typedef typename MaxElement<typename TypeListTransform<TypeList, SizeOf>::ValueT, integral_constant_less>::ValueT		MaxSize;
		typedef typename MaxElement<typename TypeListTransform<TypeList, alignment_of>::ValueT, integral_constant_less>::ValueT	MaxAlignment;

		typedef typename aligned_storage<MaxSize::Value, MaxAlignment::Value>::type Storage;

		Storage _value;

		template<typename T>
		void Ctor()											{ CheckCanContain<T>(); T* ptr = new(&_value) T(); (void)ptr; assert(ptr == &Ref<T>()); }

		template <typename T, typename P1>
		void Ctor(const P1& p1)								{ CheckCanContain<T>(); T* ptr = new(&_value) T(p1); (void)ptr; assert(ptr == &Ref<T>()); }

		template <typename T, typename P1, typename P2>
		void Ctor(const P1& p1, const P2& p2)				{ CheckCanContain<T>(); T* ptr = new(&_value) T(p1, p2); (void)ptr; assert(ptr == &Ref<T>()); }

		template <typename T, typename P1, typename P2, typename P3>
		void Ctor(const P1& p1, const P2& p2, const P3& p3)	{ CheckCanContain<T>(); T* ptr = new(&_value) T(p1, p2, p3); (void)ptr; assert(ptr == &Ref<T>()); }

		template<typename T>
		void Dtor()											{ CheckCanContain<T>(); Ref<T>().~T(); ASSERT_FAILED_FREE(); }

		template<typename T>
		T& Ref()											{ CheckCanContain<T>(); return *static_cast<T*>(static_cast<void*>(&_value)); }

		template<typename T>
		const T& Ref() const								{ CheckCanContain<T>(); return *static_cast<const T*>(static_cast<const void*>(&_value)); }

		template<typename T>
		void CheckCanContain() const						{ CompileTimeAssert<TypeListContains<TypeList, T>::Value> ERROR_TypeIsNotInList; (void)ERROR_TypeIsNotInList; }
	};

}

#endif
