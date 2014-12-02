#ifndef STINGRAY_TOOLKIT_SHARED_PTR_H
#define STINGRAY_TOOLKIT_SHARED_PTR_H


#include <sys/types.h>
#include <stdexcept>

#include <stingray/toolkit/Atomic.h>
#include <stingray/toolkit/Dummy.h>
#include <stingray/toolkit/Macro.h>
#include <stingray/toolkit/TypeInfo.h>
#include <stingray/toolkit/assert.h>
#include <stingray/toolkit/exception.h>
#include <stingray/toolkit/fatal.h>
#include <stingray/toolkit/ref_count.h>
#include <stingray/toolkit/safe_bool.h>
#include <stingray/toolkit/toolkit.h>

namespace stingray
{


#define TOOLKIT_DECLARE_PTR(ClassName) \
		typedef stingray::shared_ptr<ClassName>			ClassName##Ptr; \
		typedef stingray::weak_ptr<ClassName>			ClassName##WeakPtr

#define TOOLKIT_DECLARE_CONST_PTR(ClassName) \
		typedef stingray::shared_ptr<const ClassName>	ClassName##ConstPtr; \
		typedef stingray::weak_ptr<const ClassName>		ClassName##ConstWeakPtr


	template < typename T >
	struct shared_ptr_traits
	{
		static const bool trace_ref_counts = false;
		//static const char* get_trace_class_name();
	};


#define TOOLKIT_TRACE_SHARED_PTRS(ClassName_) \
		template < > struct shared_ptr_traits<ClassName_> \
		{ \
			static const bool trace_ref_counts = true; \
			static const char* get_trace_class_name() { return #ClassName_; }\
		}


	namespace Detail
	{
		struct IDeleter
		{
			virtual ~IDeleter() {}
			virtual void Delete() = 0;
		};


		void DoLogAddRef(const char* className, atomic_int_type refs, const void* objPtrVal, const void* sharedPtrPtrVal);
		void DoLogRelease(const char* className, atomic_int_type refs, const void* objPtrVal, const void* sharedPtrPtrVal);


		template < typename T, bool DoTrace = shared_ptr_traits<T>::trace_ref_counts >
		struct SharedPtrRefCounter
		{
			static void Create(basic_ref_count<IDeleter*>& rc, const void* objPtrVal, const void* sharedPtrPtrVal)				{ }
			static atomic_int_type AddRef(basic_ref_count<IDeleter*>& rc, const void* objPtrVal, const void* sharedPtrPtrVal)	{ return rc.add_ref(); }
			static atomic_int_type Release(basic_ref_count<IDeleter*>& rc, const void* objPtrVal, const void* sharedPtrPtrVal)	{ return rc.release(); }
			static bool ReleaseIfUnique(basic_ref_count<IDeleter*>& rc, const void* objPtrVal, const void* sharedPtrPtrVal)		{ return rc.release_if_unique(); }
		};

		template < typename T >
		struct SharedPtrRefCounter<T, true>
		{
			static void Create(basic_ref_count<IDeleter*>& rc, const void* objPtrVal, const void* sharedPtrPtrVal)
			{ Detail::DoLogAddRef(shared_ptr_traits<T>::get_trace_class_name(), rc.get(), objPtrVal, sharedPtrPtrVal); }

			static atomic_int_type AddRef(basic_ref_count<IDeleter*>& rc, const void* objPtrVal, const void* sharedPtrPtrVal)
			{
				atomic_int_type result = rc.add_ref();
				Detail::DoLogAddRef(shared_ptr_traits<T>::get_trace_class_name(), result, objPtrVal, sharedPtrPtrVal);
				return result;
			}

			static atomic_int_type Release(basic_ref_count<IDeleter*>& rc, const void* objPtrVal, const void* sharedPtrPtrVal)
			{
				atomic_int_type result = rc.release();
				Detail::DoLogRelease(shared_ptr_traits<T>::get_trace_class_name(), result, objPtrVal, sharedPtrPtrVal);
				return result;
			}

			static bool ReleaseIfUnique(basic_ref_count<IDeleter*>& rc, const void* objPtrVal, const void* sharedPtrPtrVal)
			{
				bool result = rc.release_if_unique();
				if (result)
					Detail::DoLogRelease(shared_ptr_traits<T>::get_trace_class_name(), 0, objPtrVal, sharedPtrPtrVal);
				return result;
			}
		};


		template <typename T, typename FunctorType>
		struct DeleterImpl : public IDeleter
		{
		private:
			T*			_ptr;
			FunctorType	_func;

		public:
			DeleterImpl(T* ptr, const FunctorType& func) : _ptr(TOOLKIT_REQUIRE_NOT_NULL(ptr)), _func(func)
			{ }

			virtual ~DeleterImpl()
			{ TOOLKIT_ASSERT(!_ptr); }

			virtual void Delete() { TOOLKIT_ASSERT(_ptr); _func(_ptr); _ptr = null; }
		};


		template<typename T, typename FunctorType>
		inline IDeleter* MakeNewDeleter(T* ptr, const FunctorType& func)
		{ return new DeleterImpl<T, FunctorType>(ptr, func); }

		inline IDeleter* MakeEmptyDeleter()
		{ return NULL; }
	}


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

		typedef Detail::IDeleter				DeleterType;
		typedef basic_ref_count<DeleterType*>	RefCountType;

	private:
		T*				_rawPtr;
		RefCountType	_refCount;

	private:
		inline shared_ptr(T* rawPtr, const RefCountType& refCount) :
			_rawPtr(rawPtr), _refCount(refCount)
		{ if (_rawPtr) Detail::SharedPtrRefCounter<T>::AddRef(_refCount, _rawPtr, this); }

	public:
		typedef T ValueType;


		explicit inline shared_ptr(T* rawPtr) :
			_rawPtr(rawPtr), _refCount(Detail::MakeEmptyDeleter())
		{
			if (_rawPtr)
				Detail::SharedPtrRefCounter<T>::Create(_refCount, _rawPtr, this);
			init_enable_shared_from_this(rawPtr);
		}


		template<typename Deleter>
		explicit inline shared_ptr(T* rawPtr, const Deleter& deleter) :
			_rawPtr(rawPtr), _refCount(rawPtr ? Detail::MakeNewDeleter(rawPtr, deleter) : Detail::MakeEmptyDeleter())
		{
			if (_rawPtr)
				Detail::SharedPtrRefCounter<T>::Create(_refCount, _rawPtr, this);
			init_enable_shared_from_this(rawPtr);
		}


		inline shared_ptr() :
			_rawPtr(), _refCount(null)
		{ }


		inline shared_ptr(const NullPtrType&) :
			_rawPtr(), _refCount(null)
		{ }


		template < typename U >
		inline shared_ptr(const shared_ptr<U>& other, typename EnableIf<Inherits<U, T>::Value, Dummy>::ValueT* = 0) :
			_rawPtr(other._rawPtr), _refCount(other._refCount)
		{ if (_rawPtr) Detail::SharedPtrRefCounter<T>::AddRef(_refCount, _rawPtr, this); } // Do not init enable_shared_from_this in copy ctor


		inline shared_ptr(const shared_ptr<T>& other) :
			_rawPtr(other._rawPtr), _refCount(other._refCount)
		{ if (_rawPtr) Detail::SharedPtrRefCounter<T>::AddRef(_refCount, _rawPtr, this); } // Do not init enable_shared_from_this in copy ctor


		inline ~shared_ptr()
		{
			if (!_refCount.get_ptr())
				return;

			if (Detail::SharedPtrRefCounter<T>::Release(_refCount, _rawPtr, this) == 0)
			{
				STINGRAY_ANNOTATE_HAPPENS_AFTER(_refCount.get_ptr());
				STINGRAY_ANNOTATE_RELEASE(_refCount.get_ptr());
				do_delete();
			}
			else
				STINGRAY_ANNOTATE_HAPPENS_BEFORE(_refCount.get_ptr());
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


		inline bool operator == (T* ptr) const						{ return _rawPtr == ptr; }
		inline bool operator != (T* ptr) const						{ return !(*this == ptr); }
		inline bool operator == (const shared_ptr& other) const		{ return _rawPtr == other._rawPtr; }
		inline bool operator != (const shared_ptr& other) const		{ return !(*this == other); }

		inline bool is_initialized() const							{ return _rawPtr != 0; }
		inline bool boolean_test() const							{ return is_initialized(); }

		inline weak_ptr<T> weak() const								{ return weak_ptr<T>(*this); }


		inline bool release_if_unique()
		{
			if (!_rawPtr)
				return true;

			if (!Detail::SharedPtrRefCounter<T>::ReleaseIfUnique(_refCount, _rawPtr, this))
				return false;

			do_delete();
			return true;
		}


		inline bool unique() const
		{ return !_rawPtr || _refCount.get() == 1; }


		inline size_t get_ref_count() const
		{ return _rawPtr? _refCount.get(): 0; }


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
			_refCount.swap(other._refCount);
		}


		inline T* get() const			{ return _rawPtr; }
		inline T* operator -> () const	{ check_ptr(); return _rawPtr; }
		inline T& operator * () const	{ check_ptr(); return *_rawPtr; }


		template<typename U> bool owner_before(shared_ptr<U> const& other) const
		{ return _refCount.get_ptr() < other._refCount.get_ptr(); }


		template<typename U> bool owner_before(weak_ptr<U> const& other) const
		{ return _refCount.get_ptr() < other._refCount.get_ptr(); }

	private:
		template < typename U >
		inline void init_enable_shared_from_this(const enable_shared_from_this<U>* esft) const
		{ if (esft) esft->init(*this); }


		inline void init_enable_shared_from_this(...) const
		{ }


		void do_delete()
		{
			DeleterType* deleter = _refCount.GetUserData();
			if (deleter)
			{
				if (_rawPtr)
					deleter->Delete();
				delete deleter;
			}
			else
				delete _rawPtr;

			_rawPtr = null;
			_refCount = null;
		}


		inline void check_ptr() const
		{ TOOLKIT_CHECK(_rawPtr, NullPointerException("shared_ptr<" + TypeInfo(typeid(T)).GetName() + ">")); }
	};

	/** @brief Simple weak_ptr implementation */
	template < typename T >
	class weak_ptr
	{
		template < typename U, typename V >
		friend weak_ptr<U> dynamic_pointer_cast(const weak_ptr<V>&);
		template <typename U> friend class weak_ptr;
		template <typename U> friend class shared_ptr;

		typedef Detail::IDeleter				DeleterType;
		typedef basic_ref_count<DeleterType*>	RefCountType;

	private:
		T*						_rawPtr;
		mutable RefCountType	_refCount;

	public:
		inline weak_ptr()
			: _rawPtr(), _refCount(null)
		{ }

		inline weak_ptr(const shared_ptr<T>& sharedPtr)
			: _rawPtr(sharedPtr._rawPtr), _refCount(sharedPtr._refCount)
		{ }

		inline weak_ptr(const NullPtrType&)
			: _rawPtr(), _refCount(null)
		{ }

		template < typename U >
		inline weak_ptr(const shared_ptr<U>& sharedPtr)
			: _rawPtr(sharedPtr._rawPtr), _refCount(sharedPtr._refCount)
		{ }

		template < typename U >
		inline weak_ptr(const weak_ptr<U>& other)
			: _rawPtr(other._rawPtr), _refCount(other._refCount)
		{ }

		inline shared_ptr<T> lock() const
		{
			if (!_rawPtr)
				return shared_ptr<T>();

			if (Detail::SharedPtrRefCounter<T>::AddRef(_refCount, _rawPtr, this) == 1)
			{
				if (Detail::SharedPtrRefCounter<T>::Release(_refCount, _rawPtr, this) != 0)
					TOOLKIT_FATAL("weak_ptr::lock race occured!");

				STINGRAY_ANNOTATE_HAPPENS_AFTER(_refCount.get_ptr());
				return shared_ptr<T>();
			}

			shared_ptr<T> result(_rawPtr, _refCount);
			Detail::SharedPtrRefCounter<T>::Release(_refCount, _rawPtr, this);

			return result;
		}

		inline void reset()
		{ _rawPtr = 0; _refCount = null; }

		inline size_t get_ref_count() const
		{ return _rawPtr? _refCount.get(): 0; }

		inline bool expired() const	{ return !_rawPtr || _refCount.get() == 0; }

		template<typename U> bool owner_before(shared_ptr<U> const& other) const
		{ return _refCount.get_ptr() < other._refCount.get_ptr(); }
		template<typename U> bool owner_before(weak_ptr<U> const& other) const
		{ return _refCount.get_ptr() < other._refCount.get_ptr(); }
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

		return shared_ptr<DestType>(rawDest, src._refCount);
	}

	template < typename DestType, typename SrcType >
	inline weak_ptr<DestType> dynamic_pointer_cast(const weak_ptr<SrcType>& src)
	{
		DestType* rawDest = Detail::DynamicCastHelper<DestType, SrcType>::Do(src._rawPtr);
		if (rawDest == NULL)
			return weak_ptr<DestType>();

		return weak_ptr<DestType>(rawDest, src._refCount);
	}


	template < typename T >
	struct InstanceOfTester< shared_ptr<T> >
	{
		template < typename DestType >
		static inline bool Test(const shared_ptr<const T>& ptr)
		{ return (dynamic_cast<const DestType*>(ptr.get()) != 0); }
	};


	template < typename ObjType, size_t ParamsCount >
	struct MakeShared;


	template < typename ObjType >
	shared_ptr<ObjType> make_shared() { return shared_ptr<ObjType>(new ObjType); }
	template < typename ObjType >
	shared_ptr<ObjType> make_shared_0() { return shared_ptr<ObjType>(new ObjType); }


	template < typename ObjType>
	struct MakeShared<ObjType, 0>
	{
		typedef shared_ptr<ObjType> RetType;

		shared_ptr<ObjType> operator() () const { return shared_ptr<ObjType>(new ObjType); }
	};


#define DETAIL_TOOLKIT_DECLARE_MAKE_SHARED(Size_, Typenames_, ParamsDecl_, Params_) \
	template < typename ObjType, Typenames_ > \
	shared_ptr<ObjType> make_shared_##Size_(ParamsDecl_) { return shared_ptr<ObjType>(new ObjType(Params_)); } \
	template < typename ObjType, Typenames_ > \
	shared_ptr<ObjType> make_shared(ParamsDecl_) { return shared_ptr<ObjType>(new ObjType(Params_)); } \
	template < typename ObjType > \
	struct MakeShared<ObjType, Size_> \
	{ \
		typedef shared_ptr<ObjType> RetType; \
		template < Typenames_ > \
		shared_ptr<ObjType> operator() (ParamsDecl_) const { return shared_ptr<ObjType>(new ObjType(Params_)); } \
	};


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
