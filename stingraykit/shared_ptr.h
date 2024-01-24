#ifndef STINGRAYKIT_SHARED_PTR_H
#define STINGRAYKIT_SHARED_PTR_H

// Copyright (c) 2011 - 2024, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/TypeErasure.h>
#include <stingraykit/aligned_storage.h>
#include <stingraykit/dynamic_caster.h>
#include <stingraykit/thread/atomic/AtomicInt.h>
#include <stingraykit/unique_ptr.h>

namespace stingray
{


#define STINGRAYKIT_DECLARE_PTR(ClassName) \
		using ClassName##Ptr = stingray::shared_ptr<ClassName>; \
		using ClassName##WeakPtr = stingray::weak_ptr<ClassName>

#define STINGRAYKIT_DECLARE_CONST_PTR(ClassName) \
		using ClassName##ConstPtr = stingray::shared_ptr<const ClassName>; \
		using ClassName##ConstWeakPtr = stingray::weak_ptr<const ClassName>


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
			AtomicU32::Type			_strongReferences;
			AtomicU32::Type			_weakReferences;

			ISharedPtrData() : _strongReferences(1), _weakReferences(1)
			{ }
		};


		template < typename T >
		class DefaultSharedPtrData : public ISharedPtrData
		{
		private:
			T*						_ptr;

		public:
			DefaultSharedPtrData(T* ptr) : _ptr(ptr)
			{ }

			void Dispose()
			{ CheckedDelete(_ptr); }
		};


		template < typename T, typename Deleter >
		class DeleterSharedPtrData : public ISharedPtrData
		{
			using DeleterType = typename Decay<Deleter>::ValueT;

		private:
			T*						_ptr;
			DeleterType				_deleter;

		public:
			DeleterSharedPtrData(T* ptr, Deleter&& deleter) : _ptr(ptr), _deleter(std::forward<Deleter>(deleter))
			{ }

			void Dispose()
			{ _deleter(_ptr); }
		};


		template < typename T >
		class InplaceSharedPtrData : public ISharedPtrData
		{
		private:
			StorageFor<T>			_storage;

		public:
			template < typename... Ts >
			InplaceSharedPtrData(Ts&&... args)
			{ _storage.Ctor(std::forward<Ts>(args)...); }

			void Dispose()
			{ _storage.Dtor(); }

			T* Get()
			{ return &_storage.Ref(); }
		};


		struct DisposeConcept : public function_info<void ()>
		{
			template < typename T >
			static void Apply(T& t)
			{ t.Dispose(); }
		};


		class SharedPtrImpl
		{
			STINGRAYKIT_DEFAULTCOPYABLE(SharedPtrImpl);
			STINGRAYKIT_DEFAULTMOVABLE(SharedPtrImpl);

		private:
			TypeErasure<TypeList<DisposeConcept>, ISharedPtrData>	_value;

		public:
			SharedPtrImpl()
			{ }

			template < typename DataImpl_, typename... Ts >
			DataImpl_* Allocate(Ts&&... args)
			{ return _value.Allocate<DataImpl_>(std::forward<Ts>(args)...); }

			void AddWeakReference()
			{
				if (_value.Get())
					AtomicU32::Inc(_value.Get()->_weakReferences);
			}

			void ReleaseWeakReference()
			{
				if (!_value.Get())
					return;

				u32 result = AtomicU32::Dec(_value.Get()->_weakReferences);
				if (result == 0)
				{
					STINGRAYKIT_ANNOTATE_HAPPENS_AFTER(&_value.Get()->_weakReferences);
					STINGRAYKIT_ANNOTATE_RELEASE(&_value.Get()->_weakReferences);

					_value.Free();
				}
				else
					STINGRAYKIT_ANNOTATE_HAPPENS_BEFORE(&_value.Get()->_weakReferences);
			}

			u32 GetStrongReferences() const
			{ return _value.Get() ? AtomicU32::Load(_value.Get()->_strongReferences) : 0; }

			u32 AddStrongReference()
			{
				if (!_value.Get())
					return 0;
				return AtomicU32::Inc(_value.Get()->_strongReferences);
			}

			u32 ReleaseStrongReference()
			{
				if (!_value.Get())
					return 0;

				u32 result = AtomicU32::Dec(_value.Get()->_strongReferences);
				if (result == 0)
				{
					STINGRAYKIT_ANNOTATE_HAPPENS_AFTER(&_value.Get()->_strongReferences);
					STINGRAYKIT_ANNOTATE_RELEASE(&_value.Get()->_strongReferences);
					Dispose();
				}
				else
					STINGRAYKIT_ANNOTATE_HAPPENS_BEFORE(&_value.Get()->_strongReferences);

				return result;
			}

			bool ReleaseStrongIfUnique()
			{
				if (!_value.Get())
					return false;

				u32 c = AtomicU32::CompareAndExchange(_value.Get()->_strongReferences, 1, 0);
				if (c != 1)
					return false;

				STINGRAYKIT_ANNOTATE_HAPPENS_AFTER(&_value.Get()->_strongReferences);
				STINGRAYKIT_ANNOTATE_RELEASE(&_value.Get()->_strongReferences);
				Dispose();
				return true;
			}

			u32 TryAddStrongReference()
			{
				if (!_value.Get())
					return 0;

				u32 c = AtomicU32::Load(_value.Get()->_strongReferences);
				while (c != 0)
				{
					u32 newc = AtomicU32::CompareAndExchange(_value.Get()->_strongReferences, c, c + 1);
					if (newc == c)
						return newc;
					c = newc;
				}
				return 0;
			}

			bool Before(const SharedPtrImpl& other) const
			{ return _value.Get() < other._value.Get(); }

		private:
			void Dispose()
			{
				_value.Call<DisposeConcept>();
				ReleaseWeakReference();
			}
		};


		void DoLogAddRef(const char* className, u32 refs, const void* objPtrVal, const void* sharedPtrPtrVal);
		void DoLogReleaseRef(const char* className, u32 refs, const void* objPtrVal, const void* sharedPtrPtrVal);


		template < typename T, bool DoTrace = shared_ptr_traits<T>::trace_ref_counts >
		struct SharedPtrRefCounter
		{
			static void LogAddRef(u32 referencesCount, const void* objPtrVal, const void* sharedPtrPtrVal)		{ }
			static void LogReleaseRef(u32 referencesCount, const void* objPtrVal, const void* sharedPtrPtrVal)	{ }
		};

		template < typename T >
		struct SharedPtrRefCounter<T, true>
		{
			static void LogAddRef(u32 referencesCount, const void* objPtrVal, const void* sharedPtrPtrVal)
			{ Detail::DoLogAddRef(shared_ptr_traits<T>::get_trace_class_name(), referencesCount, objPtrVal, sharedPtrPtrVal); }

			static void LogReleaseRef(u32 referencesCount, const void* objPtrVal, const void* sharedPtrPtrVal)
			{ Detail::DoLogReleaseRef(shared_ptr_traits<T>::get_trace_class_name(), referencesCount, objPtrVal, sharedPtrPtrVal); }
		};
	}


	template < typename T >
	class weak_ptr;


	template < typename T >
	struct MakeShared;


	/** @brief Simple shared_ptr implementation */
	template < typename T >
	class shared_ptr
	{
		template < typename U >
		friend class weak_ptr;

		template < typename U >
		friend class shared_ptr;

		template < typename U >
		friend struct MakeShared;

	public:
		using ValueType = T;

	private:
		T*							_rawPtr;
		Detail::SharedPtrImpl		_impl;

	private:
		shared_ptr(T* rawPtr, const Detail::SharedPtrImpl& impl, const Dummy&)
			: _rawPtr(rawPtr), _impl(impl)
		{ }

		shared_ptr(T* rawPtr, Detail::SharedPtrImpl&& impl, const Dummy&)
			: _rawPtr(rawPtr), _impl(std::move(impl))
		{ }

	public:
		explicit shared_ptr(T* rawPtr) : _rawPtr(rawPtr)
		{
			if (!_rawPtr)
				return;

			try
			{ _impl.Allocate<Detail::DefaultSharedPtrData<T> >(_rawPtr); }
			catch (...)
			{
				CheckedDelete(_rawPtr);
				throw;
			}

			LogAddRef(1);
		}

		template < typename Deleter >
		shared_ptr(T* rawPtr, Deleter&& deleter) : _rawPtr(rawPtr)
		{
			try
			{ _impl.Allocate<Detail::DeleterSharedPtrData<T, Deleter> >(_rawPtr, std::forward<Deleter>(deleter)); }
			catch (...)
			{
				deleter(_rawPtr);
				throw;
			}

			LogAddRef(1);
		}

		template < typename U >
		shared_ptr(unique_ptr<U>&& other, typename EnableIf<IsConvertible<U*, T*>::Value, Dummy>::ValueT* = 0)
			: _rawPtr(other.get())
		{
			if (!_rawPtr)
				return;

			_impl.Allocate<Detail::DefaultSharedPtrData<T> >(_rawPtr);
			other.release();

			LogAddRef(1);
		}

		shared_ptr() : _rawPtr()
		{ }

		shared_ptr(NullPtrType) : _rawPtr()
		{ }

		shared_ptr(const shared_ptr<T>& other)
			: _rawPtr(other._rawPtr), _impl(other._impl)
		{ LogAddRef(_impl.AddStrongReference()); }

		shared_ptr(shared_ptr<T>&& other)
			: _rawPtr(other._rawPtr), _impl(std::move(other._impl))
		{ other._rawPtr = null; }

		template < typename U >
		shared_ptr(const shared_ptr<U>& other, typename EnableIf<IsConvertible<U*, T*>::Value, Dummy>::ValueT* = 0)
			: _rawPtr(other._rawPtr), _impl(other._impl)
		{ LogAddRef(_impl.AddStrongReference()); }

		template < typename U >
		shared_ptr(shared_ptr<U>&& other, typename EnableIf<IsConvertible<U*, T*>::Value, Dummy>::ValueT* = 0)
			: _rawPtr(other._rawPtr), _impl(std::move(other._impl))
		{ other._rawPtr = null; }

		/// @brief: Aliasing constuctor - similar to standard one
		template < typename U >
		shared_ptr(const shared_ptr<U>& other, T* ptr)
			: _rawPtr(ptr), _impl(other._impl)
		{ LogAddRef(_impl.AddStrongReference()); }

		/// @brief: Aliasing constuctor - similar to standard one from C++20
		template < typename U >
		shared_ptr(shared_ptr<U>&& other, T* ptr)
			: _rawPtr(ptr), _impl(std::move(other._impl))
		{ other._rawPtr = null; }

		~shared_ptr()
		{ LogReleaseRef(_impl.ReleaseStrongReference()); }

		shared_ptr& operator = (const shared_ptr& other)
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

		shared_ptr& operator = (shared_ptr&& other)
		{
			shared_ptr tmp(std::move(other));
			swap(tmp);
			return *this;
		}

		template < typename U >
		typename EnableIf<IsConvertible<U*, T*>::Value, shared_ptr&>::ValueT operator = (const shared_ptr<U>& other)
		{
			shared_ptr tmp(other);
			swap(tmp);
			return *this;
		}

		template < typename U >
		typename EnableIf<IsConvertible<U*, T*>::Value, shared_ptr&>::ValueT operator = (shared_ptr<U>&& other)
		{
			shared_ptr tmp(std::move(other));
			swap(tmp);
			return *this;
		}

		template < typename U >
		typename EnableIf<IsConvertible<U*, T*>::Value, shared_ptr&>::ValueT operator = (unique_ptr<U>&& other)
		{
			shared_ptr tmp(std::move(other));
			swap(tmp);
			return *this;
		}

		bool operator == (const shared_ptr& other) const		{ return _rawPtr == other._rawPtr; }
		bool operator != (const shared_ptr& other) const		{ return !(*this == other); }

		bool is_initialized() const								{ return _rawPtr != 0; }
		explicit operator bool () const							{ return is_initialized(); }

		weak_ptr<T> weak() const								{ return weak_ptr<T>(*this); }

		bool release_if_unique()
		{
			if (!_impl.ReleaseStrongIfUnique())
				return false;

			LogReleaseRef(0);
			_rawPtr = null;
			_impl = Detail::SharedPtrImpl();
			return true;
		}

		bool unique() const
		{ return use_count() == 1; }

		size_t use_count() const
		{ return _impl.GetStrongReferences(); }

		void reset(T* ptr = 0)
		{
			shared_ptr<T> tmp(ptr);
			swap(tmp);

			// Uncomment this for tracing shared_ptrs
			//this->~shared_ptr();
			//new(this) shared_ptr(ptr);
		}

		void swap(shared_ptr<T>& other)
		{
			std::swap(_rawPtr, other._rawPtr);
			std::swap(_impl, other._impl);
		}

		T* get() const				{ return _rawPtr; }

		T* operator -> () const
		{
			check_ptr();
			return _rawPtr;
		}

		T& operator * () const
		{
			check_ptr();
			return *_rawPtr;
		}

		template < typename U >
		bool owner_before(const shared_ptr<U>& other) const
		{ return _impl.Before(other._impl); }

		template < typename U >
		bool owner_before(const weak_ptr<U>& other) const
		{ return _impl.Before(other._impl); }

	private:
		void LogAddRef(u32 referencesCount)
		{
			if (_rawPtr)
				Detail::SharedPtrRefCounter<T>::LogAddRef(referencesCount, _rawPtr, this);
		}

		void LogReleaseRef(u32 referencesCount)
		{
			if (_rawPtr)
				Detail::SharedPtrRefCounter<T>::LogReleaseRef(referencesCount, _rawPtr, this);
		}

		void check_ptr() const
		{ STINGRAYKIT_CHECK(_rawPtr, NullPointerException("shared_ptr<" + TypeInfo(typeid(T)).GetName() + ">")); }
	};


	template < typename T >
	void swap(shared_ptr<T>& lhs, shared_ptr<T>& rhs)
	{ lhs.swap(rhs); }


	/** @brief Simple weak_ptr implementation */
	template < typename T >
	class weak_ptr
	{
		template < typename U >
		friend class weak_ptr;

		template < typename U >
		friend class shared_ptr;

	private:
		T*									_rawPtr;
		mutable Detail::SharedPtrImpl		_impl;

	public:
		weak_ptr() : _rawPtr()
		{ }

		weak_ptr(NullPtrType) : _rawPtr()
		{ }

		template < typename U >
		weak_ptr(const shared_ptr<U>& other, typename EnableIf<IsConvertible<U*, T*>::Value, Dummy>::ValueT* = 0)
			: _rawPtr(other._rawPtr), _impl(other._impl)
		{ _impl.AddWeakReference(); }

		weak_ptr(const weak_ptr& other)
			: _rawPtr(other._rawPtr), _impl(other._impl)
		{ _impl.AddWeakReference(); }

		weak_ptr(weak_ptr&& other)
			: _rawPtr(other._rawPtr), _impl(std::move(other._impl))
		{ other._rawPtr = null; }

		template < typename U >
		weak_ptr(const weak_ptr<U>& other, typename EnableIf<IsConvertible<U*, T*>::Value, Dummy>::ValueT* = 0)
			: _rawPtr(other._rawPtr), _impl(other._impl)
		{ _impl.AddWeakReference(); }

		template < typename U >
		weak_ptr(weak_ptr<U>&& other, typename EnableIf<IsConvertible<U*, T*>::Value, Dummy>::ValueT* = 0)
			: _rawPtr(other._rawPtr), _impl(std::move(other._impl))
		{ other._rawPtr = null; }

		~weak_ptr()
		{ _impl.ReleaseWeakReference(); }

		weak_ptr& operator = (const weak_ptr& other)
		{
			weak_ptr tmp(other);
			swap(tmp);
			return *this;
		}

		weak_ptr& operator = (weak_ptr&& other)
		{
			weak_ptr tmp(std::move(other));
			swap(tmp);
			return *this;
		}

		template < typename U >
		typename EnableIf<IsConvertible<U*, T*>::Value, weak_ptr&>::ValueT operator = (const shared_ptr<U>& other)
		{
			weak_ptr tmp(other);
			swap(tmp);
			return *this;
		}

		template < typename U >
		typename EnableIf<IsConvertible<U*, T*>::Value, weak_ptr&>::ValueT operator = (const weak_ptr<U>& other)
		{
			weak_ptr tmp(other);
			swap(tmp);
			return *this;
		}

		template < typename U >
		typename EnableIf<IsConvertible<U*, T*>::Value, weak_ptr&>::ValueT operator = (weak_ptr<U>&& other)
		{
			weak_ptr tmp(std::move(other));
			swap(tmp);
			return *this;
		}

		shared_ptr<T> lock() const
		{
			const u32 sc = _impl.TryAddStrongReference();
			if (sc == 0)
				return shared_ptr<T>();

			if (_rawPtr)
				Detail::SharedPtrRefCounter<T>::LogAddRef(sc, _rawPtr, this);

			return shared_ptr<T>(_rawPtr, _impl, Dummy());
		}

		void reset()
		{
			weak_ptr<T> tmp;
			swap(tmp);
		}

		void swap(weak_ptr<T>& other)
		{
			std::swap(_rawPtr, other._rawPtr);
			std::swap(_impl, other._impl);
		}

		size_t use_count() const
		{ return _impl.GetStrongReferences(); }

		bool expired() const
		{ return use_count() == 0; }

		template < typename U >
		bool owner_before(const shared_ptr<U>& other) const
		{ return _impl.Before(other._impl); }

		template < typename U >
		bool owner_before(const weak_ptr<U>& other) const
		{ return _impl.Before(other._impl); }
	};


	template < typename T >
	void swap(weak_ptr<T>& lhs, weak_ptr<T>& rhs)
	{ lhs.swap(rhs); }


	template < typename SharedPtrT >
	struct IsSharedPtr : FalseType { };

	template < typename T >
	struct IsSharedPtr<shared_ptr<T> > : TrueType { };

	template < typename T >
	struct IsSharedPtr<const shared_ptr<T> > : TrueType { };


	template < typename SharedPtrT >
	struct GetSharedPtrParam;

	template < typename T >
	struct GetSharedPtrParam<shared_ptr<T> >
	{ using ValueT = T; };

	template < typename T >
	struct GetSharedPtrParam<const shared_ptr<T> >
	{ using ValueT = T; };


	template < typename WeakPtrT >
	struct GetWeakPtrParam;

	template < typename T >
	struct GetWeakPtrParam<weak_ptr<T> >
	{ using ValueT = T; };

	template < typename T >
	struct GetWeakPtrParam<const weak_ptr<T> >
	{ using ValueT = T; };


	template < typename T >
	struct ToSharedPtr
	{ using ValueT = shared_ptr<T>; };


	template < typename T >
	struct ToPointer<shared_ptr<T> >
	{ using ValueT = T*; };

	template < typename T >
	T* to_pointer(const shared_ptr<T>& ptr)
	{ return ptr.get(); }

	template < typename T >
	T* to_pointer(shared_ptr<T>& ptr)
	{ return ptr.get(); }


	namespace Detail
	{
		template < typename T >
		class WeakPtrToPointerProxy
		{
		private:
			shared_ptr<T>			_sharedPtr;

		public:
			WeakPtrToPointerProxy(const weak_ptr<T>& weakPtr)
				: _sharedPtr(STINGRAYKIT_REQUIRE_NOT_NULL(weakPtr.lock()))
			{ }

			operator T* () const { return _sharedPtr.get(); }
		};
	}

	template < typename T >
	struct ToPointer<weak_ptr<T> >
	{ using ValueT = Detail::WeakPtrToPointerProxy<T>; };

	template < typename T >
	Detail::WeakPtrToPointerProxy<T> to_pointer(const weak_ptr<T>& ptr)
	{ return ptr; }


	namespace Detail
	{
		template < typename SrcPtr_, typename DstPtr_ >
		struct DynamicCastImpl<SrcPtr_, DstPtr_, typename EnableIf<IsSharedPtr<SrcPtr_>::Value && IsSharedPtr<DstPtr_>::Value, void>::ValueT>
		{
			static DstPtr_ Do(const SrcPtr_& src)
			{
				typename DstPtr_::ValueType* dst = PointersCaster<typename SrcPtr_::ValueType, typename DstPtr_::ValueType>::Do(src.get());
				return dst ? DstPtr_(src, dst) : DstPtr_();
			}
		};


		template < typename Src_, typename Dst_ >
		struct DynamicCastImpl<Src_, Dst_, typename EnableIf<IsSharedPtr<Src_>::Value != IsSharedPtr<Dst_>::Value, void>::ValueT>
		{
			// Explicitly prohibit casting if one of the types is a pointer and another one is not
		};


		template < typename Src_ >
		class DynamicCasterImpl<Src_, typename EnableIf<IsSharedPtr<Src_>::Value, void>::ValueT>
		{
		private:
			Src_					_src;

		public:
			explicit DynamicCasterImpl(const Src_& src) : _src(src)
			{ }

			template < typename Dst_ >
			operator shared_ptr<Dst_> () const
			{ return DynamicCast<shared_ptr<Dst_>, Src_>(_src); }
		};
	}


	template < typename T >
	struct InstanceOfTester< shared_ptr<T> >
	{
		template < typename DestType >
		static bool Test(const shared_ptr<const T>& ptr)
		{ return dynamic_cast<const DestType*>(ptr.get()); }
	};


	template < typename ObjType >
	struct MakeShared
	{
		using RetType = shared_ptr<ObjType>;

		template < typename... Ts >
		shared_ptr<ObjType> operator () (Ts&&... args) const
		{
			(void)sizeof(new ObjType(std::forward<Ts>(args)...)); // Testing the type for being abstract

			Detail::SharedPtrImpl impl;
			Detail::InplaceSharedPtrData<ObjType>* data = impl.Allocate<Detail::InplaceSharedPtrData<ObjType>>(std::forward<Ts>(args)...);

			const shared_ptr<ObjType> result(data->Get(), std::move(impl), Dummy());
			Detail::SharedPtrRefCounter<ObjType>::LogAddRef(1, result.get(), &result);

			return result;
		}
	};


	template < typename ObjType, typename... Ts >
	shared_ptr<ObjType> make_shared_ptr(Ts&&... args)
	{ return MakeShared<ObjType>()(std::forward<Ts>(args)...); }


	template < typename T >
	struct IsNullable<shared_ptr<T> > : public TrueType { };

	template < typename T >
	struct IsNullable<weak_ptr<T> > : public TrueType { };

}

#endif
