#ifndef __GS_DVRLIB_TOOLKIT_LIGHT_SHARED_PTR_H__
#define __GS_DVRLIB_TOOLKIT_LIGHT_SHARED_PTR_H__


#include <sys/types.h>
#include <stdexcept>

#include <dvrlib/toolkit/Atomic.h>
#include <dvrlib/toolkit/Macro.h>
#include <dvrlib/toolkit/exception.h>
#include <dvrlib/toolkit/ref_count.h>
#include <dvrlib/toolkit/safe_bool.h>
#include <dvrlib/toolkit/toolkit.h>

namespace dvrlib
{

	
#define TOOLKIT_DECLARE_LIGHT_PTR(ClassName) \
		typedef dvrlib::light_shared_ptr<ClassName>				ClassName##LightPtr; \
		typedef dvrlib::light_shared_ptr<const ClassName>		ClassName##LightConstPtr; \
		typedef dvrlib::light_weak_ptr<ClassName>				ClassName##LightWeakPtr; \
		typedef dvrlib::light_weak_ptr<const ClassName>			ClassName##LightConstWeakPtr


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
		FORCE_INLINE light_shared_ptr(const RefCountT& refCount)
			: _refCount(refCount)
		{ if (!_refCount.IsNull()) _refCount.add_ref(); }

		static FORCE_INLINE RefCountT init_ref_count(T* rawPtr)
		{ return rawPtr ? RefCountT(rawPtr) : RefCountT(null); }

	public:
		typedef T ValueType;
		
		explicit FORCE_INLINE light_shared_ptr(T* rawPtr) 
			: _refCount(init_ref_count(rawPtr))
		{ }

		light_shared_ptr()
			: _refCount(null)
		{ }

		light_shared_ptr(const NullPtrType&)
			: _refCount(null)
		{ }

		FORCE_INLINE light_shared_ptr(const light_shared_ptr<T>& other)
			: _refCount(other._refCount)
		{ if (!_refCount.IsNull()) _refCount.add_ref(); }

		FORCE_INLINE ~light_shared_ptr()
		{
			T* raw_ptr = get();
			if (!_refCount.IsNull() && _refCount.release() == 0)
				delete raw_ptr;
		}


		FORCE_INLINE light_shared_ptr<T>& operator = (const light_shared_ptr<T>& other)
		{
			light_shared_ptr<T> tmp(other);
			swap(tmp);
			return *this;
		}

		FORCE_INLINE bool operator == (T* ptr) const						{ return get() == ptr; }
		FORCE_INLINE bool operator != (T* ptr) const						{ return !(*this == ptr); }
		FORCE_INLINE bool operator == (const light_shared_ptr<T>& other) const	{ return other == get(); }
		FORCE_INLINE bool operator != (const light_shared_ptr<T>& other) const	{ return !(*this == other); }
		FORCE_INLINE bool boolean_test() const { return get() != 0; }

		FORCE_INLINE bool unique() const
		{ return !get() || _refCount.get() == 1; }

		FORCE_INLINE size_t get_ref_count() const
		{ return get()? _refCount.get(): 0; }

		FORCE_INLINE void reset()
		{
			light_shared_ptr<T> tmp;
			swap(tmp);
		}

		FORCE_INLINE void reset(T* ptr)
		{
			light_shared_ptr<T> tmp(ptr);
			swap(tmp);
		}

		FORCE_INLINE void swap(light_shared_ptr<T>& other)
		{ _refCount.swap(other._refCount); }

		FORCE_INLINE T* get() const			{ return _refCount.IsNull() ? 0 : _refCount.GetUserData(); } // TODO
		FORCE_INLINE T* operator -> () const	{ check_ptr(); return get(); }
		FORCE_INLINE T& operator * () const	{ check_ptr(); return *get(); }

	private:
		FORCE_INLINE void check_ptr() const
		{ 
			if (!get())
				TOOLKIT_THROW(NullPointerException());
		}
	};

	template < typename T >
	class light_weak_ptr
	{
		typedef basic_ref_count<T*>		RefCountT;

	private:
		RefCountT	_refCount;

	private:
		FORCE_INLINE light_weak_ptr(const ref_count& refCount)
			: _refCount(refCount)
		{ }

	public:
		FORCE_INLINE light_weak_ptr()
			: _refCount(null)
		{ }

		FORCE_INLINE light_weak_ptr(const light_shared_ptr<T>& sharedPtr)
			: _refCount(sharedPtr._refCount)
		{ }

		FORCE_INLINE light_weak_ptr(const NullPtrType&)
			: _refCount(null)
		{ }

		FORCE_INLINE light_shared_ptr<T> lock() const
		{ 
			if (expired())
				return light_shared_ptr<T>();

			return light_shared_ptr<T>(_refCount); 
		}
		
		FORCE_INLINE size_t get_ref_count() const
		{ return !_refCount.IsNull() ? _refCount.get(): 0; }

		FORCE_INLINE bool expired() const	{ return _refCount.IsNull() || _refCount.get() == 0; }
	};

	
	/*! \cond GS_INTERNAL *
	template < typename SharedPtrT >
	struct GetSharedPtrParam;

	template < typename T >
	struct GetSharedPtrParam<light_shared_ptr<T> >
	{ typedef T	ValueT; };

	template < typename T >
	struct GetSharedPtrParam<const light_shared_ptr<T> >
	{ typedef T	ValueT; };


	template < typename T >
	struct ToSharedPtr
	{ typedef light_shared_ptr<T>	ValueT; };


	template < typename T > 
	FORCE_INLINE T* to_pointer(const light_shared_ptr<T>& ptr) { return ptr.get(); }

	
	namespace Detail
	{

		template < typename T >
		light_shared_ptr<T> RequireNotNull(const light_shared_ptr<T>& ptr, const char* expr, const char* file, size_t line, const char* func)
		{ if (!ptr) throw dvrlib::Detail::MakeException(NullPointerException(expr), file, line, func); else return ptr; }


		template < typename T >
		class WeakPtrToPointerProxy
		{
		private:
			light_shared_ptr<T>	_sharedPtr;
			
		public:
			WeakPtrToPointerProxy(const light_weak_ptr<T>& weakPtr)
				: _sharedPtr(TOOLKIT_REQUIRE_NOT_NULL(weakPtr.lock()))
			{}

			operator T* () const { return _sharedPtr.get(); }
		};
	}

	template < typename T > 
	FORCE_INLINE Detail::WeakPtrToPointerProxy<T> to_pointer(const light_weak_ptr<T>& ptr) { return ptr; }


	template < typename DestType, typename SrcType >
	FORCE_INLINE light_shared_ptr<DestType> dynamic_pointer_cast(const light_shared_ptr<SrcType>& src)
	{ 
		DestType* rawDest = dynamic_cast<DestType*>(src.get());
		if (rawDest == NULL)
			return light_shared_ptr<DestType>();

		return light_shared_ptr<DestType>(rawDest, src._refCount);
	}

	template < typename DestType, typename SrcType >
	FORCE_INLINE light_weak_ptr<DestType> dynamic_pointer_cast(const light_weak_ptr<SrcType>& src)
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
		static FORCE_INLINE bool Test(const light_shared_ptr<const T>& ptr)
		{ return (dynamic_cast<const DestType*>(ptr.get()) != 0); }
	};


	template < typename ObjType >
	light_shared_ptr<ObjType> make_shared() { return light_shared_ptr<ObjType>(new ObjType); }
	template < typename ObjType >
	light_shared_ptr<ObjType> make_shared_0() { return light_shared_ptr<ObjType>(new ObjType); }


#define DETAIL_TOOLKIT_DECLARE_MAKE_LIGHT_SHARED(Size_, Typenames_, ParamsDecl_, Params_) \
	template < typename ObjType, Typenames_ > \
	light_shared_ptr<ObjType> make_shared_##Size_(ParamsDecl_) { return light_shared_ptr<ObjType>(new ObjType(Params_)); } \
	template < typename ObjType, Typenames_ > \
	light_shared_ptr<ObjType> make_shared(ParamsDecl_) { return light_shared_ptr<ObjType>(new ObjType(Params_)); }


#define TY typename
#define P_(N) const T##N& p##N
	
	DETAIL_TOOLKIT_DECLARE_MAKE_LIGHT_SHARED(1, MK_PARAM(TY T1), MK_PARAM(P_(1)), MK_PARAM(p1))
	DETAIL_TOOLKIT_DECLARE_MAKE_LIGHT_SHARED(2, MK_PARAM(TY T1, TY T2), MK_PARAM(P_(1), P_(2)), MK_PARAM(p1, p2))
	DETAIL_TOOLKIT_DECLARE_MAKE_LIGHT_SHARED(3, MK_PARAM(TY T1, TY T2, TY T3), MK_PARAM(P_(1), P_(2), P_(3)), MK_PARAM(p1, p2, p3))
	DETAIL_TOOLKIT_DECLARE_MAKE_LIGHT_SHARED(4, MK_PARAM(TY T1, TY T2, TY T3, TY T4), MK_PARAM(P_(1), P_(2), P_(3), P_(4)), MK_PARAM(p1, p2, p3, p4))
	DETAIL_TOOLKIT_DECLARE_MAKE_LIGHT_SHARED(5, MK_PARAM(TY T1, TY T2, TY T3, TY T4, TY T5), MK_PARAM(P_(1), P_(2), P_(3), P_(4), P_(5)), MK_PARAM(p1, p2, p3, p4, p5))
	DETAIL_TOOLKIT_DECLARE_MAKE_LIGHT_SHARED(6, MK_PARAM(TY T1, TY T2, TY T3, TY T4, TY T5, TY T6), MK_PARAM(P_(1), P_(2), P_(3), P_(4), P_(5), P_(6)), MK_PARAM(p1, p2, p3, p4, p5, p6))
	DETAIL_TOOLKIT_DECLARE_MAKE_LIGHT_SHARED(7, MK_PARAM(TY T1, TY T2, TY T3, TY T4, TY T5, TY T6, TY T7), MK_PARAM(P_(1), P_(2), P_(3), P_(4), P_(5), P_(6), P_(7)), MK_PARAM(p1, p2, p3, p4, p5, p6, p7))
	DETAIL_TOOLKIT_DECLARE_MAKE_LIGHT_SHARED(8, MK_PARAM(TY T1, TY T2, TY T3, TY T4, TY T5, TY T6, TY T7, TY T8), MK_PARAM(P_(1), P_(2), P_(3), P_(4), P_(5), P_(6), P_(7), P_(8)), MK_PARAM(p1, p2, p3, p4, p5, p6, p7, p8))
	DETAIL_TOOLKIT_DECLARE_MAKE_LIGHT_SHARED(9, MK_PARAM(TY T1, TY T2, TY T3, TY T4, TY T5, TY T6, TY T7, TY T8, TY T9), MK_PARAM(P_(1), P_(2), P_(3), P_(4), P_(5), P_(6), P_(7), P_(8), P_(9)), MK_PARAM(p1, p2, p3, p4, p5, p6, p7, p8, p9))
	DETAIL_TOOLKIT_DECLARE_MAKE_LIGHT_SHARED(10, MK_PARAM(TY T1, TY T2, TY T3, TY T4, TY T5, TY T6, TY T7, TY T8, TY T9, TY T10), MK_PARAM(P_(1), P_(2), P_(3), P_(4), P_(5), P_(6), P_(7), P_(8), P_(9), P_(10)), MK_PARAM(p1, p2, p3, p4, p5, p6, p7, p8, p9, p10))

#undef P_
#undef TY
#undef DETAIL_TOOLKIT_DECLARE_MAKE_LIGHT_SHARED
	*! \endcond */

}


#endif
