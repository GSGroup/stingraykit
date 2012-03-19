#ifndef __GS_DVRLIB_TOOLKIT_SHARED_PTR_H__
#define __GS_DVRLIB_TOOLKIT_SHARED_PTR_H__


#include <sys/types.h>
#include <stdexcept>

#include <stingray/toolkit/Atomic.h>
#include <stingray/toolkit/Macro.h>
#include <stingray/toolkit/exception.h>
#include <stingray/toolkit/ref_count.h>
#include <stingray/toolkit/safe_bool.h>
#include <stingray/toolkit/toolkit.h>

namespace stingray
{


#define TOOLKIT_DECLARE_PTR(ClassName) \
		typedef stingray::shared_ptr<ClassName>			ClassName##Ptr; \
		typedef stingray::shared_ptr<const ClassName>		ClassName##ConstPtr; \
		typedef stingray::weak_ptr<ClassName>				ClassName##WeakPtr; \
		typedef stingray::weak_ptr<const ClassName>		ClassName##ConstWeakPtr


	template < typename T >
	class enable_shared_from_this;

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

		template < typename U, typename V >
		friend shared_ptr<U> dynamic_pointer_cast(const shared_ptr<V>&);

	private:
		T*				_rawPtr;
		ref_count		_refCount;

	private:
		FORCE_INLINE shared_ptr(T* rawPtr, const ref_count& refCount)
			: _rawPtr(rawPtr), _refCount(refCount)
		{ if (_rawPtr) _refCount.add_ref(); }

	public:
		typedef T ValueType;

		explicit FORCE_INLINE shared_ptr(T* rawPtr)
			: _rawPtr(rawPtr)
		{
			if (rawPtr != 0)
				init_enable_shared_from_this(rawPtr);
		}

		shared_ptr()
			: _rawPtr(), _refCount(null)
		{ }

		shared_ptr(const NullPtrType&)
			: _rawPtr(), _refCount(null)
		{ }

		weak_ptr<T> weak() const { return weak_ptr<T>(*this); }

		template < typename U >
		FORCE_INLINE shared_ptr(const shared_ptr<U>& other)
			: _rawPtr(other._rawPtr), _refCount(other._refCount)
		{ if (_rawPtr) _refCount.add_ref(); } // Do not init enable_shared_from_this in copy ctor

		FORCE_INLINE shared_ptr(const shared_ptr<T>& other)
			: _rawPtr(other._rawPtr), _refCount(other._refCount)
		{ if (_rawPtr) _refCount.add_ref(); } // Do not init enable_shared_from_this in copy ctor

		FORCE_INLINE ~shared_ptr()
		{
			if (_rawPtr && _refCount.release() == 0)
				delete _rawPtr;
		}


		FORCE_INLINE shared_ptr<T>& operator = (const shared_ptr<T>& other)
		{
			shared_ptr<T> tmp(other);
			swap(tmp);
			return *this;
		}

		FORCE_INLINE bool operator == (T* ptr) const						{ return _rawPtr == ptr; }
		FORCE_INLINE bool operator != (T* ptr) const						{ return !(*this == ptr); }
		FORCE_INLINE bool operator == (const shared_ptr<T>& other) const	{ return other == _rawPtr; }
		FORCE_INLINE bool operator != (const shared_ptr<T>& other) const	{ return !(*this == other); }
		FORCE_INLINE bool boolean_test() const { return _rawPtr != 0; }

		FORCE_INLINE bool unique() const
		{ return !_rawPtr || _refCount.get() == 1; }

		FORCE_INLINE size_t get_ref_count() const
		{ return _rawPtr? _refCount.get(): 0; }

		FORCE_INLINE void reset(T* ptr = 0)
		{
			shared_ptr<T> tmp(ptr);
			swap(tmp);
		}

		FORCE_INLINE void swap(shared_ptr<T>& other)
		{
			std::swap(_rawPtr, other._rawPtr);
			_refCount.swap(other._refCount);
		}

		FORCE_INLINE T* get() const			{ return _rawPtr; }
		FORCE_INLINE T* operator -> () const	{ check_ptr(); return _rawPtr; }
		FORCE_INLINE T& operator * () const	{ check_ptr(); return *_rawPtr; }

	private:
		template < typename U >
		FORCE_INLINE void init_enable_shared_from_this(const enable_shared_from_this<U>* esft) const
		{ esft->init(*this); }

		inline void init_enable_shared_from_this(...) const
		{ }


		FORCE_INLINE void check_ptr() const
		{
			if (!_rawPtr)
				TOOLKIT_THROW(NullPointerException());
		}
	};

	/** @brief Simple weak_ptr implementation */
	template < typename T >
	class weak_ptr
	{
		template < typename U, typename V >
		friend weak_ptr<U> dynamic_pointer_cast(const weak_ptr<V>&);
		template <typename U> friend class weak_ptr;

	private:
		T*			_rawPtr;
		ref_count	_refCount;

	private:
		FORCE_INLINE weak_ptr(T* rawPtr, const ref_count& refCount)
			: _rawPtr(rawPtr), _refCount(refCount)
		{ }

	public:
		FORCE_INLINE weak_ptr()
			: _rawPtr(), _refCount(null)
		{ }

		FORCE_INLINE weak_ptr(const shared_ptr<T>& sharedPtr)
			: _rawPtr(sharedPtr._rawPtr), _refCount(sharedPtr._refCount)
		{ }

		FORCE_INLINE weak_ptr(const NullPtrType&)
			: _rawPtr(), _refCount(null)
		{ }

		template < typename U >
		FORCE_INLINE weak_ptr(const shared_ptr<U>& sharedPtr)
			: _rawPtr(sharedPtr._rawPtr), _refCount(sharedPtr._refCount)
		{ }

		template < typename U >
		FORCE_INLINE weak_ptr(const weak_ptr<U>& other)
			: _rawPtr(other._rawPtr), _refCount(other._refCount)
		{ }

		FORCE_INLINE shared_ptr<T> lock() const
		{
			if (expired())
				return shared_ptr<T>();

			return shared_ptr<T>(_rawPtr, _refCount);
		}

		FORCE_INLINE size_t get_ref_count() const
		{ return _rawPtr? _refCount.get(): 0; }

		FORCE_INLINE bool expired() const	{ return !_rawPtr || _refCount.get() == 0; }
	};


	/*! \cond GS_INTERNAL */

	template < typename SharedPtrT >
	struct GetSharedPtrParam;

	template < typename T >
	struct GetSharedPtrParam<shared_ptr<T> >
	{ typedef T	ValueT; };

	template < typename T >
	struct GetSharedPtrParam<const shared_ptr<T> >
	{ typedef T	ValueT; };


	template < typename T >
	struct ToSharedPtr
	{ typedef shared_ptr<T>	ValueT; };


	template < typename T >
	struct ToPointerType<shared_ptr<T> >
	{ typedef T* ValueT; };

	template < typename T >
	FORCE_INLINE T* to_pointer(const shared_ptr<T>& ptr) { return ptr.get(); }


	namespace Detail
	{
		template < typename T >
		class WeakPtrToPointerProxy
		{
		private:
			shared_ptr<T>	_sharedPtr;

		public:
			WeakPtrToPointerProxy(const weak_ptr<T>& weakPtr)
				: _sharedPtr(TOOLKIT_REQUIRE_NOT_NULL(weakPtr.lock()))
			{}

			operator T* () const { return _sharedPtr.get(); }
			operator bool () const { return _sharedPtr; }
		};
	}

	template < typename T >
	struct ToPointerType<weak_ptr<T> >
	{ typedef Detail::WeakPtrToPointerProxy<T> ValueT; };

	template < typename T >
	FORCE_INLINE Detail::WeakPtrToPointerProxy<T> to_pointer(const weak_ptr<T>& ptr) { return ptr; }


	template < typename DestType, typename SrcType >
	FORCE_INLINE shared_ptr<DestType> dynamic_pointer_cast(const shared_ptr<SrcType>& src)
	{
		DestType* rawDest = dynamic_cast<DestType*>(src.get());
		if (rawDest == NULL)
			return shared_ptr<DestType>();

		return shared_ptr<DestType>(rawDest, src._refCount);
	}

	template < typename DestType, typename SrcType >
	FORCE_INLINE weak_ptr<DestType> dynamic_pointer_cast(const weak_ptr<SrcType>& src)
	{
		DestType* rawDest = dynamic_cast<DestType*>(src._rawPtr);
		if (rawDest == NULL)
			return weak_ptr<DestType>();

		return weak_ptr<DestType>(rawDest, src._refCount);
	}


	template < typename T >
	struct InstanceOfTester< shared_ptr<T> >
	{
		template < typename DestType >
		static FORCE_INLINE bool Test(const shared_ptr<const T>& ptr)
		{ return (dynamic_cast<const DestType*>(ptr.get()) != 0); }
	};


	template < typename ObjType >
	shared_ptr<ObjType> make_shared() { return shared_ptr<ObjType>(new ObjType); }
	template < typename ObjType >
	shared_ptr<ObjType> make_shared_0() { return shared_ptr<ObjType>(new ObjType); }


#define DETAIL_TOOLKIT_DECLARE_MAKE_SHARED(Size_, Typenames_, ParamsDecl_, Params_) \
	template < typename ObjType, Typenames_ > \
	shared_ptr<ObjType> make_shared_##Size_(ParamsDecl_) { return shared_ptr<ObjType>(new ObjType(Params_)); } \
	template < typename ObjType, Typenames_ > \
	shared_ptr<ObjType> make_shared(ParamsDecl_) { return shared_ptr<ObjType>(new ObjType(Params_)); }


#define TY typename
#define P_(N) const T##N& p##N

	DETAIL_TOOLKIT_DECLARE_MAKE_SHARED(1, MK_PARAM(TY T1), MK_PARAM(P_(1)), MK_PARAM(p1))
	DETAIL_TOOLKIT_DECLARE_MAKE_SHARED(2, MK_PARAM(TY T1, TY T2), MK_PARAM(P_(1), P_(2)), MK_PARAM(p1, p2))
	DETAIL_TOOLKIT_DECLARE_MAKE_SHARED(3, MK_PARAM(TY T1, TY T2, TY T3), MK_PARAM(P_(1), P_(2), P_(3)), MK_PARAM(p1, p2, p3))
	DETAIL_TOOLKIT_DECLARE_MAKE_SHARED(4, MK_PARAM(TY T1, TY T2, TY T3, TY T4), MK_PARAM(P_(1), P_(2), P_(3), P_(4)), MK_PARAM(p1, p2, p3, p4))
	DETAIL_TOOLKIT_DECLARE_MAKE_SHARED(5, MK_PARAM(TY T1, TY T2, TY T3, TY T4, TY T5), MK_PARAM(P_(1), P_(2), P_(3), P_(4), P_(5)), MK_PARAM(p1, p2, p3, p4, p5))
	DETAIL_TOOLKIT_DECLARE_MAKE_SHARED(6, MK_PARAM(TY T1, TY T2, TY T3, TY T4, TY T5, TY T6), MK_PARAM(P_(1), P_(2), P_(3), P_(4), P_(5), P_(6)), MK_PARAM(p1, p2, p3, p4, p5, p6))
	DETAIL_TOOLKIT_DECLARE_MAKE_SHARED(7, MK_PARAM(TY T1, TY T2, TY T3, TY T4, TY T5, TY T6, TY T7), MK_PARAM(P_(1), P_(2), P_(3), P_(4), P_(5), P_(6), P_(7)), MK_PARAM(p1, p2, p3, p4, p5, p6, p7))
	DETAIL_TOOLKIT_DECLARE_MAKE_SHARED(8, MK_PARAM(TY T1, TY T2, TY T3, TY T4, TY T5, TY T6, TY T7, TY T8), MK_PARAM(P_(1), P_(2), P_(3), P_(4), P_(5), P_(6), P_(7), P_(8)), MK_PARAM(p1, p2, p3, p4, p5, p6, p7, p8))
	DETAIL_TOOLKIT_DECLARE_MAKE_SHARED(9, MK_PARAM(TY T1, TY T2, TY T3, TY T4, TY T5, TY T6, TY T7, TY T8, TY T9), MK_PARAM(P_(1), P_(2), P_(3), P_(4), P_(5), P_(6), P_(7), P_(8), P_(9)), MK_PARAM(p1, p2, p3, p4, p5, p6, p7, p8, p9))
	DETAIL_TOOLKIT_DECLARE_MAKE_SHARED(10, MK_PARAM(TY T1, TY T2, TY T3, TY T4, TY T5, TY T6, TY T7, TY T8, TY T9, TY T10), MK_PARAM(P_(1), P_(2), P_(3), P_(4), P_(5), P_(6), P_(7), P_(8), P_(9), P_(10)), MK_PARAM(p1, p2, p3, p4, p5, p6, p7, p8, p9, p10))

#undef P_
#undef TY
#undef DETAIL_TOOLKIT_DECLARE_MAKE_SHARED
	/*! \endcond */

}


#endif
