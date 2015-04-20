#ifndef STINGRAYKIT_SHARED_PTR_H
#define STINGRAYKIT_SHARED_PTR_H

// Copyright (c) 2011 - 2015, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.


#include <sys/types.h>
#include <stdexcept>

#include <stingraykit/Atomic.h>
#include <stingraykit/Dummy.h>
#include <stingraykit/Macro.h>
#include <stingraykit/TypeErasure.h>
#include <stingraykit/TypeInfo.h>
#include <stingraykit/assert.h>
#include <stingraykit/exception.h>
#include <stingraykit/fatal.h>
#include <stingraykit/safe_bool.h>
#include <stingraykit/toolkit.h>

namespace stingray
{


#define STINGRAYKIT_DECLARE_PTR(ClassName) \
		typedef stingray::shared_ptr<ClassName>			ClassName##Ptr; \
		typedef stingray::weak_ptr<ClassName>			ClassName##WeakPtr

#define STINGRAYKIT_DECLARE_CONST_PTR(ClassName) \
		typedef stingray::shared_ptr<const ClassName>	ClassName##ConstPtr; \
		typedef stingray::weak_ptr<const ClassName>		ClassName##ConstWeakPtr


	template < typename T >
	struct shared_ptr_traits
	{
		static const bool trace_ref_counts = false;
		//static const char* get_trace_class_name();
	};


#define STINGRAYKIT_TRACE_SHARED_PTRS(ClassName_) \
		template < > struct shared_ptr_traits<ClassName_> \
		{ \
			static const bool trace_ref_counts = true; \
			static const char* get_trace_class_name() { return #ClassName_; }\
		}


	namespace Detail
	{
		struct ISharedPtrData : public TypeErasureBase
		{
			atomic_int_type		_strongReferences;
			atomic_int_type		_weakReferences;

			ISharedPtrData() : _strongReferences(1), _weakReferences(1)
			{ }
		};


		template<typename T>
		class DefaultSharedPtrData : public ISharedPtrData
		{
		private:
			T*	_ptr;

		public:
			DefaultSharedPtrData(T* ptr) : _ptr(ptr)
			{ }

			void Dispose()
			{ delete _ptr; }
		};


		template<typename T, typename Deleter>
		class DeleterSharedPtrData : public ISharedPtrData
		{
		private:
			T*		_ptr;
			Deleter	_deleter;

		public:
			DeleterSharedPtrData(T* ptr, const Deleter& deleter) : _ptr(ptr), _deleter(deleter)
			{ }

			void Dispose()
			{ _deleter(_ptr); }
		};


		struct DisposeConcept : public function_info<void, TypeList_0>
		{
			template<typename T>
			static void Apply(T& t)
			{ t.Dispose(); }
		};


#define DETAIL_STINGRAYKIT_SHAREDPTRIMPL_ALLOCATE(N_) \
		template<typename DataImpl_, STINGRAYKIT_REPEAT(N_, STINGRAYKIT_TEMPLATE_PARAM_DECL, T)> \
		DataImpl_* Allocate(STINGRAYKIT_REPEAT(N_, STINGRAYKIT_FUNCTION_PARAM_DECL, T)) \
		{ return _value.Allocate<DataImpl_>(STINGRAYKIT_REPEAT(N_, STINGRAYKIT_FUNCTION_PARAM_USAGE, ~)); }


		class SharedPtrImpl
		{
			TypeErasure<TypeList<DisposeConcept>::type, ISharedPtrData>	_value;

		public:
			inline SharedPtrImpl()
			{ }

			template<typename DataImpl_>
			DataImpl_* Allocate()
			{ return _value.Allocate<DataImpl_>(); }

			DETAIL_STINGRAYKIT_SHAREDPTRIMPL_ALLOCATE(1)
			DETAIL_STINGRAYKIT_SHAREDPTRIMPL_ALLOCATE(2)
			DETAIL_STINGRAYKIT_SHAREDPTRIMPL_ALLOCATE(3)
			DETAIL_STINGRAYKIT_SHAREDPTRIMPL_ALLOCATE(4)
			DETAIL_STINGRAYKIT_SHAREDPTRIMPL_ALLOCATE(5)
			DETAIL_STINGRAYKIT_SHAREDPTRIMPL_ALLOCATE(6)
			DETAIL_STINGRAYKIT_SHAREDPTRIMPL_ALLOCATE(7)
			DETAIL_STINGRAYKIT_SHAREDPTRIMPL_ALLOCATE(8)
			DETAIL_STINGRAYKIT_SHAREDPTRIMPL_ALLOCATE(9)
			DETAIL_STINGRAYKIT_SHAREDPTRIMPL_ALLOCATE(10)

			void Dispose()								{ return _value.Call<DisposeConcept>(); }
			void Destroy()								{ _value.Free(); }

			atomic_int_type GetWeakReferences() const	{ return _value.Get()->_weakReferences; }
			atomic_int_type AddWeakReference()			{ return Atomic::Inc(_value.Get()->_weakReferences); }
			atomic_int_type ReleaseWeakReference()		{ return Atomic::Dec(_value.Get()->_weakReferences); }

			atomic_int_type GetStrongReferences() const	{ return _value.Get()->_strongReferences; }
			atomic_int_type AddStrongReference()		{ return Atomic::Inc(_value.Get()->_strongReferences); }
			atomic_int_type ReleaseStrongReference()	{ return Atomic::Dec(_value.Get()->_strongReferences); }

			bool ReleaseStrongIfUnique()				{ return Atomic::CompareAndExchange(_value.Get()->_strongReferences, 1, 0) == 1; }
			atomic_int_type TryAddStrongReference()
			{
				atomic_int_type c = Atomic::Load(_value.Get()->_strongReferences);
				while (c != 0)
				{
					atomic_int_type newc = Atomic::CompareAndExchange(_value.Get()->_strongReferences, c, c + 1);
					if (newc == c)
						return newc;
					c = newc;
				}
				return 0;
			}

			const ISharedPtrData* get_ptr() const		{ return _value.Get(); }
		};


		void DoLogAddRef(const char* className, atomic_int_type refs, const void* objPtrVal, const void* sharedPtrPtrVal);
		void DoLogReleaseRef(const char* className, atomic_int_type refs, const void* objPtrVal, const void* sharedPtrPtrVal);


		template < typename T, bool DoTrace = shared_ptr_traits<T>::trace_ref_counts >
		struct SharedPtrRefCounter
		{
			static void LogAddRef(atomic_int_type referencesCount, const void* objPtrVal, const void* sharedPtrPtrVal)		{ }
			static void LogReleaseRef(atomic_int_type referencesCount, const void* objPtrVal, const void* sharedPtrPtrVal)	{ }
		};

		template < typename T >
		struct SharedPtrRefCounter<T, true>
		{
			static void LogAddRef(atomic_int_type referencesCount, const void* objPtrVal, const void* sharedPtrPtrVal)
			{ Detail::DoLogAddRef(shared_ptr_traits<T>::get_trace_class_name(), referencesCount, objPtrVal, sharedPtrPtrVal); }

			static void LogReleaseRef(atomic_int_type referencesCount, const void* objPtrVal, const void* sharedPtrPtrVal)
			{ Detail::DoLogReleaseRef(shared_ptr_traits<T>::get_trace_class_name(), referencesCount, objPtrVal, sharedPtrPtrVal); }
		};
	}


	template < typename T >
	class weak_ptr;


	/** @brief Simple shared_ptr implementation */
	template < typename T >
	class shared_ptr : public safe_bool<shared_ptr<T> >
	{
		template < typename U >
		friend class weak_ptr;

		template < typename U >
		friend class shared_ptr;

	private:
		T*						_rawPtr;
		Detail::SharedPtrImpl	_impl;

	private:
		inline shared_ptr(T* rawPtr, const Detail::SharedPtrImpl& impl) :
			_rawPtr(rawPtr), _impl(impl)
		{ }

	public:
		typedef T ValueType;


		explicit inline shared_ptr(T* rawPtr) : _rawPtr(rawPtr)
		{
			if (!_rawPtr)
				return;

			_impl.Allocate<Detail::DefaultSharedPtrData<T> >(rawPtr);
			Detail::SharedPtrRefCounter<T>::LogAddRef(1, _rawPtr, this);

			init_enable_shared_from_this(_rawPtr);
		}


		template<typename Deleter>
		inline shared_ptr(T* rawPtr, const Deleter& deleter) : _rawPtr(rawPtr)
		{
			if (!_rawPtr)
				return;

			_impl.Allocate<Detail::DeleterSharedPtrData<T, Deleter> >(rawPtr, deleter);
			Detail::SharedPtrRefCounter<T>::LogAddRef(1, _rawPtr, this);

			init_enable_shared_from_this(_rawPtr);
		}


		inline shared_ptr() : _rawPtr()
		{ }


		inline shared_ptr(const NullPtrType&) : _rawPtr()
		{ }


		inline shared_ptr(const shared_ptr<T>& other) :
			_rawPtr(other._rawPtr), _impl(other._impl)
		{ AddRef(); }


		template < typename U >
		inline shared_ptr(const shared_ptr<U>& other, typename EnableIf<Inherits<U, T>::Value, Dummy>::ValueT* = 0) :
			_rawPtr(other._rawPtr), _impl(other._impl)
		{ AddRef(); }


		/// @brief: Aliasing constuctor - similar to standard one
		template < typename U >
		inline shared_ptr(const shared_ptr<U>& other, T* ptr) :
			_rawPtr(ptr), _impl(other._impl)
		{ AddRef(); }


		inline ~shared_ptr()
		{
			if (!_rawPtr)
				return;

			atomic_int_type sc = _impl.ReleaseStrongReference();
			Detail::SharedPtrRefCounter<T>::LogReleaseRef(sc, _rawPtr, this);

			if (sc == 0)
			{
				STINGRAYKIT_ANNOTATE_HAPPENS_AFTER(_impl.get_ptr());
				do_delete();
			}
			else
				STINGRAYKIT_ANNOTATE_HAPPENS_BEFORE(_impl.get_ptr());
		}


		inline shared_ptr& operator = (const shared_ptr& other)
		{
			shared_ptr tmp(other);
			swap(tmp);

			// Uncomment this for tracing shared_ptrs
			//if (other == *this)
				//return *this;

			//this->~shared_ptr();
			//new(this) shared_ptr(other);

			return *this;
		}


		inline bool operator == (const T* ptr) const				{ return _rawPtr == ptr; }
		inline bool operator != (const T* ptr) const				{ return !(*this == ptr); }
		inline bool operator == (const shared_ptr& other) const		{ return _rawPtr == other._rawPtr; }
		inline bool operator != (const shared_ptr& other) const		{ return !(*this == other); }

		inline bool is_initialized() const							{ return _rawPtr != 0; }
		inline bool boolean_test() const							{ return is_initialized(); }

		inline weak_ptr<T> weak() const								{ return weak_ptr<T>(*this); }


		inline bool release_if_unique()
		{
			if (!_rawPtr)
				return true;

			bool result = _impl.ReleaseStrongIfUnique();
			if (!result)
				return false;

			Detail::SharedPtrRefCounter<T>::LogReleaseRef(0, _rawPtr, this);

			STINGRAYKIT_ANNOTATE_HAPPENS_AFTER(_impl.get_ptr());
			do_delete();

			return true;
		}


		inline bool unique() const
		{ return !_rawPtr || _impl.GetStrongReferences() == 1; }


		inline size_t get_ref_count() const
		{ return _rawPtr? _impl.GetStrongReferences(): 0; }


		inline void reset(T* ptr = 0)
		{
			shared_ptr<T> tmp(ptr);
			swap(tmp);

			// Uncomment this for tracing shared_ptrs
			//this->~shared_ptr();
			//new(this) shared_ptr(ptr);
		}


		inline void swap(shared_ptr<T>& other)
		{
			std::swap(_rawPtr, other._rawPtr);
			std::swap(_impl, other._impl);
		}


		inline T* get() const			{ return _rawPtr; }
		inline T* operator -> () const	{ check_ptr(); return _rawPtr; }
		inline T& operator * () const	{ check_ptr(); return *_rawPtr; }


		template<typename U> bool owner_before(shared_ptr<U> const& other) const
		{ return _impl.get_ptr() < other._impl.get_ptr(); }


		template<typename U> bool owner_before(weak_ptr<U> const& other) const
		{ return _impl.get_ptr() < other._impl.get_ptr(); }

	private:
		inline void init_enable_shared_from_this(...) const
		{ }


		void AddRef()
		{
			if (!_rawPtr)
				return;

			atomic_int_type c = _impl.AddStrongReference();
			Detail::SharedPtrRefCounter<T>::LogAddRef(c, _rawPtr, this);
		}


		void do_delete()
		{
			_impl.Dispose();
			_rawPtr = null;

			atomic_int_type wc = _impl.ReleaseWeakReference();
			if (wc == 0)
			{
				STINGRAYKIT_ANNOTATE_HAPPENS_AFTER(_impl.get_ptr());
				STINGRAYKIT_ANNOTATE_RELEASE(_impl.get_ptr());

				_impl.Destroy();
			}
			else
				STINGRAYKIT_ANNOTATE_HAPPENS_BEFORE(_impl.get_ptr());

			_impl = Detail::SharedPtrImpl();
		}


		inline void check_ptr() const
		{ STINGRAYKIT_CHECK(_rawPtr, NullPointerException("shared_ptr<" + TypeInfo(typeid(T)).GetName() + ">")); }
	};


	/** @brief Simple weak_ptr implementation */
	template < typename T >
	class weak_ptr
	{
		template <typename U> friend class weak_ptr;
		template <typename U> friend class shared_ptr;

	private:
		T*								_rawPtr;
		mutable Detail::SharedPtrImpl	_impl;

	public:
		inline weak_ptr() : _rawPtr()
		{ }

		inline weak_ptr(const NullPtrType&) : _rawPtr()
		{ }

		inline weak_ptr(const shared_ptr<T>& sharedPtr) :
			_rawPtr(sharedPtr._rawPtr), _impl(sharedPtr._impl)
		{ AddRef(); }

		template < typename U >
		inline weak_ptr(const shared_ptr<U>& sharedPtr) :
			_rawPtr(sharedPtr._rawPtr), _impl(sharedPtr._impl)
		{ AddRef(); }

		inline weak_ptr(const weak_ptr& other) :
			_rawPtr(other._rawPtr), _impl(other._impl)
		{ AddRef(); }

		template < typename U >
		inline weak_ptr(const weak_ptr<U>& other) :
			_rawPtr(other._rawPtr), _impl(other._impl)
		{ AddRef(); }

		inline ~weak_ptr()
		{
			if (!_rawPtr)
				return;

			atomic_int_type wc = _impl.ReleaseWeakReference();
			if (wc == 0)
			{
				STINGRAYKIT_ANNOTATE_HAPPENS_AFTER(_impl.get_ptr());
				STINGRAYKIT_ANNOTATE_RELEASE(_impl.get_ptr());

				_impl.Destroy();
			}
			else
				STINGRAYKIT_ANNOTATE_HAPPENS_BEFORE(_impl.get_ptr());
		}

		inline weak_ptr& operator = (const weak_ptr& other)
		{
			weak_ptr tmp(other);
			swap(tmp);
			return *this;
		}

		inline shared_ptr<T> lock() const
		{
			if (!_rawPtr)
				return shared_ptr<T>();

			atomic_int_type sc = _impl.TryAddStrongReference();
			if (sc == 0)
			{
				STINGRAYKIT_ANNOTATE_HAPPENS_AFTER(_impl.get_ptr()); // TODO: Check whether this is necessary
				return shared_ptr<T>();
			}
			Detail::SharedPtrRefCounter<T>::LogAddRef(sc, _rawPtr, this);

			shared_ptr<T> result(_rawPtr, _impl);
			return result;
		}

		inline void reset()
		{
			weak_ptr<T> tmp;
			swap(tmp);
		}

		inline void swap(weak_ptr<T>& other)
		{
			std::swap(_rawPtr, other._rawPtr);
			std::swap(_impl, other._impl);
		}

		inline size_t get_ref_count() const
		{ return _rawPtr? _impl.GetStrongReferences(): 0; }

		inline bool expired() const	{ return !_rawPtr || _impl.GetStrongReferences() == 0; }

		template<typename U> bool owner_before(shared_ptr<U> const& other) const
		{ return _impl.get_ptr() < other._impl.get_ptr(); }
		template<typename U> bool owner_before(weak_ptr<U> const& other) const
		{ return _impl.get_ptr() < other._impl.get_ptr(); }

	private:
		void AddRef()
		{
			if (!_rawPtr)
				return;

			_impl.AddWeakReference();
		}
	};


	template < typename SharedPtrT >
	struct IsSharedPtr
	{ static const bool Value = false; };

	template < typename T >
	struct IsSharedPtr<shared_ptr<T> >
	{ static const bool Value = true; };


	template < typename SharedPtrT >
	struct GetSharedPtrParam;

	template < typename T >
	struct GetSharedPtrParam<shared_ptr<T> >
	{ typedef T	ValueT; };

	template < typename T >
	struct GetSharedPtrParam<const shared_ptr<T> >
	{ typedef T	ValueT; };


	template < typename WeakPtrT >
	struct GetWeakPtrParam;

	template < typename T >
	struct GetWeakPtrParam<weak_ptr<T> >
	{ typedef T	ValueT; };

	template < typename T >
	struct GetWeakPtrParam<const weak_ptr<T> >
	{ typedef T	ValueT; };


	template < typename T >
	struct ToSharedPtr
	{ typedef shared_ptr<T>	ValueT; };


	template < typename T >
	struct ToPointerType<shared_ptr<T> >
	{ typedef T* ValueT; };

	template < typename T >
	inline T* to_pointer(const shared_ptr<T>& ptr) { return ptr.get(); }

	template < typename T >
	inline T* to_pointer(shared_ptr<T>& ptr) { return ptr.get(); }


	namespace Detail
	{
		template < typename T >
		class WeakPtrToPointerProxy
		{
		private:
			shared_ptr<T>	_sharedPtr;

		public:
			WeakPtrToPointerProxy(const weak_ptr<T>& weakPtr)
				: _sharedPtr(STINGRAYKIT_REQUIRE_NOT_NULL(weakPtr.lock()))
			{}

			operator T* () const { return _sharedPtr.get(); }
			operator bool () const { return _sharedPtr; }
		};
	}

	template < typename T >
	struct ToPointerType<weak_ptr<T> >
	{ typedef Detail::WeakPtrToPointerProxy<T> ValueT; };

	template < typename T >
	inline Detail::WeakPtrToPointerProxy<T> to_pointer(const weak_ptr<T>& ptr) { return ptr; }


	namespace Detail
	{
		template < typename DestType, typename SrcType, bool UseImplicit = Inherits<SrcType, DestType>::Value >
		struct DynamicCastHelper
		{
			static DestType* Do(SrcType* src)
			{
				CompileTimeAssert<!IsPointer<DestType>::Value> ERROR__pointer_to_pointer_dynamic_cast;
				(void)ERROR__pointer_to_pointer_dynamic_cast;
				return dynamic_cast<DestType*>(src);
			}
		};

		template < typename DestType, typename SrcType >
		struct DynamicCastHelper<DestType, SrcType, true>
		{
			static DestType* Do(SrcType* src)
			{ return src; }
		};
	}


	template < typename DestType, typename SrcType >
	inline shared_ptr<DestType> dynamic_pointer_cast(const shared_ptr<SrcType>& src)
	{
		DestType* rawDest = Detail::DynamicCastHelper<DestType, SrcType>::Do(src.get());
		if (rawDest == NULL)
			return shared_ptr<DestType>();

		return shared_ptr<DestType>(src, rawDest);
	}


	template < typename T >
	struct InstanceOfTester< shared_ptr<T> >
	{
		template < typename DestType >
		static inline bool Test(const shared_ptr<const T>& ptr)
		{ return (dynamic_cast<const DestType*>(ptr.get()) != 0); }
	};


	template < typename ObjType >
	shared_ptr<ObjType> make_shared() { return shared_ptr<ObjType>(new ObjType); }
	template < typename ObjType >
	shared_ptr<ObjType> make_shared_0() { return shared_ptr<ObjType>(new ObjType); }


#define DETAIL_STINGRAYKIT_DECLARE_MAKE_SHARED(Size_, Typenames_, ParamsDecl_, Params_) \
	template < typename ObjType, Typenames_ > \
	shared_ptr<ObjType> make_shared(ParamsDecl_) { return shared_ptr<ObjType>(new ObjType(Params_)); }


#define TY typename
#define P_(N) const T##N& p##N

	DETAIL_STINGRAYKIT_DECLARE_MAKE_SHARED(1, MK_PARAM(TY T1), MK_PARAM(P_(1)), MK_PARAM(p1))
	DETAIL_STINGRAYKIT_DECLARE_MAKE_SHARED(2, MK_PARAM(TY T1, TY T2), MK_PARAM(P_(1), P_(2)), MK_PARAM(p1, p2))
	DETAIL_STINGRAYKIT_DECLARE_MAKE_SHARED(3, MK_PARAM(TY T1, TY T2, TY T3), MK_PARAM(P_(1), P_(2), P_(3)), MK_PARAM(p1, p2, p3))
	DETAIL_STINGRAYKIT_DECLARE_MAKE_SHARED(4, MK_PARAM(TY T1, TY T2, TY T3, TY T4), MK_PARAM(P_(1), P_(2), P_(3), P_(4)), MK_PARAM(p1, p2, p3, p4))
	DETAIL_STINGRAYKIT_DECLARE_MAKE_SHARED(5, MK_PARAM(TY T1, TY T2, TY T3, TY T4, TY T5), MK_PARAM(P_(1), P_(2), P_(3), P_(4), P_(5)), MK_PARAM(p1, p2, p3, p4, p5))
	DETAIL_STINGRAYKIT_DECLARE_MAKE_SHARED(6, MK_PARAM(TY T1, TY T2, TY T3, TY T4, TY T5, TY T6), MK_PARAM(P_(1), P_(2), P_(3), P_(4), P_(5), P_(6)), MK_PARAM(p1, p2, p3, p4, p5, p6))
	DETAIL_STINGRAYKIT_DECLARE_MAKE_SHARED(7, MK_PARAM(TY T1, TY T2, TY T3, TY T4, TY T5, TY T6, TY T7), MK_PARAM(P_(1), P_(2), P_(3), P_(4), P_(5), P_(6), P_(7)), MK_PARAM(p1, p2, p3, p4, p5, p6, p7))
	DETAIL_STINGRAYKIT_DECLARE_MAKE_SHARED(8, MK_PARAM(TY T1, TY T2, TY T3, TY T4, TY T5, TY T6, TY T7, TY T8), MK_PARAM(P_(1), P_(2), P_(3), P_(4), P_(5), P_(6), P_(7), P_(8)), MK_PARAM(p1, p2, p3, p4, p5, p6, p7, p8))
	DETAIL_STINGRAYKIT_DECLARE_MAKE_SHARED(9, MK_PARAM(TY T1, TY T2, TY T3, TY T4, TY T5, TY T6, TY T7, TY T8, TY T9), MK_PARAM(P_(1), P_(2), P_(3), P_(4), P_(5), P_(6), P_(7), P_(8), P_(9)), MK_PARAM(p1, p2, p3, p4, p5, p6, p7, p8, p9))
	DETAIL_STINGRAYKIT_DECLARE_MAKE_SHARED(10, MK_PARAM(TY T1, TY T2, TY T3, TY T4, TY T5, TY T6, TY T7, TY T8, TY T9, TY T10), MK_PARAM(P_(1), P_(2), P_(3), P_(4), P_(5), P_(6), P_(7), P_(8), P_(9), P_(10)), MK_PARAM(p1, p2, p3, p4, p5, p6, p7, p8, p9, p10))

#undef P_
#undef TY
#undef DETAIL_STINGRAYKIT_DECLARE_MAKE_SHARED


#define DETAIL_STINGRAYKIT_MAKE_SHARED_FUNCTOR_OPERATOR(N_) \
	template<STINGRAYKIT_REPEAT(N_, STINGRAYKIT_TEMPLATE_PARAM_DECL, T)> \
	shared_ptr<ObjType> operator() (STINGRAYKIT_REPEAT(N_, STINGRAYKIT_FUNCTION_PARAM_DECL, T)) const \
	{ return shared_ptr<ObjType>(new ObjType(STINGRAYKIT_REPEAT(N_, STINGRAYKIT_FUNCTION_PARAM_USAGE, ~))); }

	template <typename ObjType>
	struct MakeShared
	{
		typedef shared_ptr<ObjType> RetType;

		shared_ptr<ObjType> operator() () const
		{ return shared_ptr<ObjType>(new ObjType()); }
		DETAIL_STINGRAYKIT_MAKE_SHARED_FUNCTOR_OPERATOR(1);
		DETAIL_STINGRAYKIT_MAKE_SHARED_FUNCTOR_OPERATOR(2);
		DETAIL_STINGRAYKIT_MAKE_SHARED_FUNCTOR_OPERATOR(3);
		DETAIL_STINGRAYKIT_MAKE_SHARED_FUNCTOR_OPERATOR(4);
		DETAIL_STINGRAYKIT_MAKE_SHARED_FUNCTOR_OPERATOR(5);
		DETAIL_STINGRAYKIT_MAKE_SHARED_FUNCTOR_OPERATOR(6);
	};

#undef DETAIL_STINGRAYKIT_MAKE_SHARED_FUNCTOR_OPERATOR


	template<typename T>
	struct owner_less;

	template<typename T>
	struct owner_less<weak_ptr<T> > : public std::binary_function<weak_ptr<T>, weak_ptr<T>, bool>
	{
		bool operator()(const weak_ptr<T>& t1, const weak_ptr<T>& t2) const		{ return t1.owner_before(t2); }
		bool operator()(const weak_ptr<T>& t1, const shared_ptr<T>& t2) const	{ return t1.owner_before(t2); }
		bool operator()(const shared_ptr<T>& t1, const weak_ptr<T>& t2) const	{ return t1.owner_before(t2); }
	};

	template<typename T>
	struct owner_less<shared_ptr<T> > : public std::binary_function<shared_ptr<T>, shared_ptr<T>, bool>
	{
		bool operator()(const shared_ptr<T>& t1, const shared_ptr<T>& t2) const	{ return t1.owner_before(t2); }
		bool operator()(const weak_ptr<T>& t1, const shared_ptr<T>& t2) const	{ return t1.owner_before(t2); }
		bool operator()(const shared_ptr<T>& t1, const weak_ptr<T>& t2) const	{ return t1.owner_before(t2); }
	};

}


#endif
