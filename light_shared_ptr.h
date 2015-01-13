#ifndef STINGRAYKIT_LIGHT_SHARED_PTR_H
#define STINGRAYKIT_LIGHT_SHARED_PTR_H


#include <sys/types.h>
#include <stdexcept>

#include <stingray/toolkit/Macro.h>
#include <stingray/toolkit/exception.h>
#include <stingray/toolkit/ref_count.h>
#include <stingray/toolkit/safe_bool.h>
#include <stingray/toolkit/toolkit.h>

namespace stingray
{


#define STINGRAYKIT_DECLARE_LIGHT_PTR(ClassName) \
		typedef stingray::light_shared_ptr<ClassName>				ClassName##LightPtr; \
		typedef stingray::light_shared_ptr<const ClassName>		ClassName##LightConstPtr; \
		typedef stingray::light_weak_ptr<ClassName>				ClassName##LightWeakPtr; \
		typedef stingray::light_weak_ptr<const ClassName>			ClassName##LightConstWeakPtr


	template < typename T >
	class light_weak_ptr;


	template < typename T >
	class light_shared_ptr : public safe_bool<light_shared_ptr<T> >
	{
		template < typename U >
		friend class light_weak_ptr;

		typedef basic_ref_count<T*>		RefCountT;

	private:
		RefCountT		_refCount;

	private:
		inline light_shared_ptr(const RefCountT& refCount)
			: _refCount(refCount)
		{ if (!_refCount.IsNull()) _refCount.add_ref(); }

		static inline RefCountT init_ref_count(T* rawPtr)
		{ return rawPtr ? RefCountT(rawPtr) : RefCountT(null); }

	public:
		typedef T ValueType;

		explicit inline light_shared_ptr(T* rawPtr)
			: _refCount(init_ref_count(rawPtr))
		{ }

		light_shared_ptr()
			: _refCount(null)
		{ }

		light_shared_ptr(const NullPtrType&)
			: _refCount(null)
		{ }

		inline light_shared_ptr(const light_shared_ptr<T>& other)
			: _refCount(other._refCount)
		{ if (!_refCount.IsNull()) _refCount.add_ref(); }

		inline ~light_shared_ptr()
		{
			T* raw_ptr = get();
			if (!_refCount.IsNull() && _refCount.release() == 0)
				delete raw_ptr;
		}


		inline light_shared_ptr<T>& operator = (const light_shared_ptr<T>& other)
		{
			light_shared_ptr<T> tmp(other);
			swap(tmp);
			return *this;
		}

		inline bool operator == (T* ptr) const						{ return get() == ptr; }
		inline bool operator != (T* ptr) const						{ return !(*this == ptr); }
		inline bool operator == (const light_shared_ptr<T>& other) const	{ return other == get(); }
		inline bool operator != (const light_shared_ptr<T>& other) const	{ return !(*this == other); }
		inline bool boolean_test() const { return get() != 0; }

		inline bool unique() const
		{ return !get() || _refCount.get() == 1; }

		inline size_t get_ref_count() const
		{ return get()? _refCount.get(): 0; }

		inline void reset()
		{
			light_shared_ptr<T> tmp;
			swap(tmp);
		}

		inline void reset(T* ptr)
		{
			light_shared_ptr<T> tmp(ptr);
			swap(tmp);
		}

		inline void swap(light_shared_ptr<T>& other)
		{ _refCount.swap(other._refCount); }

		inline T* get() const				{ return _refCount.IsNull() ? 0 : _refCount.GetUserData(); }
		inline T* operator -> () const	{ check_ptr(); return get(); }
		inline T& operator * () const		{ check_ptr(); return *get(); }

	private:
		inline void check_ptr() const
		{ STINGRAYKIT_CHECK(get(), NullPointerException()); }
	};

	template < typename T >
	class light_weak_ptr
	{
		typedef basic_ref_count<T*>		RefCountT;

	private:
		RefCountT	_refCount;

	private:
		inline light_weak_ptr(const ref_count& refCount)
			: _refCount(refCount)
		{ }

	public:
		inline light_weak_ptr()
			: _refCount(null)
		{ }

		inline light_weak_ptr(const light_shared_ptr<T>& sharedPtr)
			: _refCount(sharedPtr._refCount)
		{ }

		inline light_weak_ptr(const NullPtrType&)
			: _refCount(null)
		{ }

		inline light_shared_ptr<T> lock() const
		{
			if (expired())
				return light_shared_ptr<T>();

			return light_shared_ptr<T>(_refCount);
		}

		inline size_t get_ref_count() const
		{ return !_refCount.IsNull() ? _refCount.get(): 0; }

		inline bool expired() const	{ return _refCount.IsNull() || _refCount.get() == 0; }
	};


	template < typename SharedPtrT >
	struct GetSharedPtrParam;

	template < typename T >
	struct GetSharedPtrParam<light_shared_ptr<T> >
	{ typedef T	ValueT; };

	template < typename T >
	struct GetSharedPtrParam<const light_shared_ptr<T> >
	{ typedef T	ValueT; };

	template < typename T >
	inline T* to_pointer(const light_shared_ptr<T>& ptr) { return ptr.get(); }


	namespace Detail
	{

		template < typename T >
		class LightWeakPtrToPointerProxy
		{
		private:
			light_shared_ptr<T>	_sharedPtr;

		public:
			LightWeakPtrToPointerProxy(const light_weak_ptr<T>& weakPtr)
				: _sharedPtr(STINGRAYKIT_REQUIRE_NOT_NULL(weakPtr.lock()))
			{}

			operator T* () const { return _sharedPtr.get(); }
		};
	}

	template < typename T >
	inline Detail::LightWeakPtrToPointerProxy<T> to_pointer(const light_weak_ptr<T>& ptr) { return ptr; }


	template < typename DestType, typename SrcType >
	inline light_shared_ptr<DestType> dynamic_pointer_cast(const light_shared_ptr<SrcType>& src)
	{
		DestType* rawDest = dynamic_cast<DestType*>(src.get());
		if (rawDest == NULL)
			return light_shared_ptr<DestType>();

		return light_shared_ptr<DestType>(rawDest, src._refCount);
	}

	template < typename DestType, typename SrcType >
	inline light_weak_ptr<DestType> dynamic_pointer_cast(const light_weak_ptr<SrcType>& src)
	{
		DestType* rawDest = dynamic_cast<DestType*>(src._rawPtr);
		if (rawDest == NULL)
			return light_weak_ptr<DestType>();

		return light_weak_ptr<DestType>(rawDest, src._refCount);
	}


	template < typename T >
	struct InstanceOfTester< light_shared_ptr<T> >
	{
		template < typename DestType >
		static inline bool Test(const light_shared_ptr<const T>& ptr)
		{ return (dynamic_cast<const DestType*>(ptr.get()) != 0); }
	};


	template < typename ObjType >
	light_shared_ptr<ObjType> make_light_shared() { return light_shared_ptr<ObjType>(new ObjType); }
	template < typename ObjType >
	light_shared_ptr<ObjType> make_light_shared_0() { return light_shared_ptr<ObjType>(new ObjType); }


#define DETAIL_STINGRAYKIT_DECLARE_MAKE_LIGHT_SHARED(Size_, Typenames_, ParamsDecl_, Params_) \
	template < typename ObjType, Typenames_ > \
	light_shared_ptr<ObjType> make_light_shared_##Size_(ParamsDecl_) { return light_shared_ptr<ObjType>(new ObjType(Params_)); } \
	template < typename ObjType, Typenames_ > \
	light_shared_ptr<ObjType> make_light_shared(ParamsDecl_) { return light_shared_ptr<ObjType>(new ObjType(Params_)); }


#define TY typename
#define P_(N) const T##N& p##N

	DETAIL_STINGRAYKIT_DECLARE_MAKE_LIGHT_SHARED(1, MK_PARAM(TY T1), MK_PARAM(P_(1)), MK_PARAM(p1))
	DETAIL_STINGRAYKIT_DECLARE_MAKE_LIGHT_SHARED(2, MK_PARAM(TY T1, TY T2), MK_PARAM(P_(1), P_(2)), MK_PARAM(p1, p2))
	DETAIL_STINGRAYKIT_DECLARE_MAKE_LIGHT_SHARED(3, MK_PARAM(TY T1, TY T2, TY T3), MK_PARAM(P_(1), P_(2), P_(3)), MK_PARAM(p1, p2, p3))
	DETAIL_STINGRAYKIT_DECLARE_MAKE_LIGHT_SHARED(4, MK_PARAM(TY T1, TY T2, TY T3, TY T4), MK_PARAM(P_(1), P_(2), P_(3), P_(4)), MK_PARAM(p1, p2, p3, p4))
	DETAIL_STINGRAYKIT_DECLARE_MAKE_LIGHT_SHARED(5, MK_PARAM(TY T1, TY T2, TY T3, TY T4, TY T5), MK_PARAM(P_(1), P_(2), P_(3), P_(4), P_(5)), MK_PARAM(p1, p2, p3, p4, p5))
	DETAIL_STINGRAYKIT_DECLARE_MAKE_LIGHT_SHARED(6, MK_PARAM(TY T1, TY T2, TY T3, TY T4, TY T5, TY T6), MK_PARAM(P_(1), P_(2), P_(3), P_(4), P_(5), P_(6)), MK_PARAM(p1, p2, p3, p4, p5, p6))
	DETAIL_STINGRAYKIT_DECLARE_MAKE_LIGHT_SHARED(7, MK_PARAM(TY T1, TY T2, TY T3, TY T4, TY T5, TY T6, TY T7), MK_PARAM(P_(1), P_(2), P_(3), P_(4), P_(5), P_(6), P_(7)), MK_PARAM(p1, p2, p3, p4, p5, p6, p7))
	DETAIL_STINGRAYKIT_DECLARE_MAKE_LIGHT_SHARED(8, MK_PARAM(TY T1, TY T2, TY T3, TY T4, TY T5, TY T6, TY T7, TY T8), MK_PARAM(P_(1), P_(2), P_(3), P_(4), P_(5), P_(6), P_(7), P_(8)), MK_PARAM(p1, p2, p3, p4, p5, p6, p7, p8))
	DETAIL_STINGRAYKIT_DECLARE_MAKE_LIGHT_SHARED(9, MK_PARAM(TY T1, TY T2, TY T3, TY T4, TY T5, TY T6, TY T7, TY T8, TY T9), MK_PARAM(P_(1), P_(2), P_(3), P_(4), P_(5), P_(6), P_(7), P_(8), P_(9)), MK_PARAM(p1, p2, p3, p4, p5, p6, p7, p8, p9))
	DETAIL_STINGRAYKIT_DECLARE_MAKE_LIGHT_SHARED(10, MK_PARAM(TY T1, TY T2, TY T3, TY T4, TY T5, TY T6, TY T7, TY T8, TY T9, TY T10), MK_PARAM(P_(1), P_(2), P_(3), P_(4), P_(5), P_(6), P_(7), P_(8), P_(9), P_(10)), MK_PARAM(p1, p2, p3, p4, p5, p6, p7, p8, p9, p10))

#undef P_
#undef TY
#undef DETAIL_STINGRAYKIT_DECLARE_MAKE_LIGHT_SHARED

}


#endif
