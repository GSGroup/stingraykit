#ifndef STINGRAY_TOOLKIT_MULTISTORAGEFOR_H
#define STINGRAY_TOOLKIT_MULTISTORAGEFOR_H


#include <stingray/toolkit/aligned_storage.h>
#include <stingray/toolkit/TypeList.h>

namespace stingray
{

	template<typename TypeList>
	struct MultiStorageFor
	{
		typedef typename MaxElement<typename TypeListTransform<TypeList, SizeOf>::ValueT>::ValueT		MaxSize;
		typedef typename MaxElement<typename TypeListTransform<TypeList, alignment_of>::ValueT>::ValueT	MaxAlignment;

		typedef typename aligned_storage<MaxSize::Value, MaxAlignment::Value>::type Storage;

		Storage _value;

		template<typename T>
		void Ctor()											{ CheckCanContain<T>(); T* ptr = new(&_value) T(); assert(ptr == &Ref<T>()); }

		template <typename T, typename P1>
		void Ctor(const P1& p1)								{ CheckCanContain<T>(); T* ptr = new(&_value) T(p1); assert(ptr == &Ref<T>()); }

		template <typename T, typename P1, typename P2>
		void Ctor(const P1& p1, const P2& p2)				{ CheckCanContain<T>(); T* ptr = new(&_value) T(p1, p2); assert(ptr == &Ref<T>()); }

		template <typename T, typename P1, typename P2, typename P3>
		void Ctor(const P1& p1, const P2& p2, const P3& p3)	{ CheckCanContain<T>(); T* ptr = new(&_value) T(p1, p2, p3); assert(ptr == &Ref<T>()); }

		template<typename T>
		void Dtor()											{ CheckCanContain<T>(); Ref<T>().~T(); }

		template<typename T>
		T& Ref()											{ CheckCanContain<T>(); return *static_cast<T*>(static_cast<void*>(&_value)); }

		template<typename T>
		const T& Ref() const								{ CheckCanContain<T>(); return *static_cast<const T*>(static_cast<const void*>(&_value)); }

		template<typename T>
		void CheckCanContain() const						{ CompileTimeAssert<TypeListContains<TypeList, T>::Value> ERROR_TypeIsNotInList; (void)ERROR_TypeIsNotInList; }
	};

}

#endif
