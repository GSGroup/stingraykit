#ifndef STINGRAYKIT_DYNAMIC_CASTER_H
#define STINGRAYKIT_DYNAMIC_CASTER_H


#include <stingray/toolkit/MetaProgramming.h>
#include <stingray/toolkit/shared_ptr.h>
#include <stingray/toolkit/exception.h>
#include <stingray/toolkit/Dummy.h>
#include <stingray/toolkit/TypeInfo.h>


namespace stingray
{


	template < typename T >
	class DynamicCaster
	{
	private:
		T*		_ptr;

	public:
		DynamicCaster(T* ptr, const Dummy&) : _ptr(ptr) { }
		DynamicCaster(T& ref, const Dummy&, const Dummy&) : _ptr(&ref) { }

		template < typename U > operator U* () const
		{ return Detail::DynamicCastHelper<U, T>::Do(_ptr); }

		operator const DynamicCaster&() const { return *this; } // workaround for an old GCC bug

		template < typename U > operator U& () const
		{
			CompileTimeAssert<!SameType<typename Deconst<U>::ValueT, DynamicCaster>::Value> ERROR__gcc_3x_bug;
			(void)ERROR__gcc_3x_bug;
			U* result_ptr = Detail::DynamicCastHelper<U, T>::Do(_ptr);
			if (!result_ptr)
				STINGRAYKIT_THROW(std::bad_cast());
			return *result_ptr;
		}

		template < typename U > operator const U& () const
		{
			CompileTimeAssert<!SameType<typename Deconst<U>::ValueT, DynamicCaster>::Value> ERROR__gcc_3x_bug;
			(void)ERROR__gcc_3x_bug;
			const U* result_ptr = Detail::DynamicCastHelper<const U, T>::Do(_ptr);
			if (!result_ptr)
				STINGRAYKIT_THROW(std::bad_cast());
			return *result_ptr;
		}
	};


	template < typename T >
	class SharedDynamicCaster
	{
	private:
		shared_ptr<T>		_ptr;

	public:
		SharedDynamicCaster(const shared_ptr<T>& ptr, const Dummy&) : _ptr(ptr) { }

		operator const SharedDynamicCaster&() const { return *this; } // workaround for an old GCC bug

		template < typename U > operator shared_ptr<U> () const { return dynamic_pointer_cast<U>(_ptr); }
		/*
		template < typename U > operator U* () const { return dynamic_cast<U*>(_ptr.get()); }
		template < typename U > operator U& () const
		{
			CompileTimeAssert<!SameType<U, SharedDynamicCaster>::Value> ERROR__old_gcc_bug; (void)ERROR__old_gcc_bug;
			CompileTimeAssert<!Is1ParamTemplate<shared_ptr, typename Deconst<U>::Type>::Value> ERROR__test; (void)ERROR__test; // =(((
			return dynamic_cast<U&>(*_ptr.get());
		}
		*/
	};


	template < typename T > DynamicCaster<T>
		dynamic_caster(T* ptr)
	{ return DynamicCaster<T>(ptr, Dummy()); }


	template < typename T > DynamicCaster<typename EnableIf<!IsSharedPtr<T>::Value, T>::ValueT>
		dynamic_caster(T& ref)
	{ return DynamicCaster<T>(ref, Dummy(), Dummy()); }


	template < typename T > SharedDynamicCaster<T>
		dynamic_caster(const shared_ptr<T>& shPtr)
	{ return SharedDynamicCaster<T>(shPtr, Dummy()); }

	namespace Detail
	{


		template < typename SourceType, typename DestinationType >
		struct Caster;


		template < typename SourceType, typename DestinationType >
		struct Caster<SourceType*, DestinationType*>
		{
			static DestinationType* Do(SourceType* source)
			{ return dynamic_cast<DestinationType*>(source); }
		};


		template < typename SourceType, typename DestinationType >
		struct Caster<shared_ptr<SourceType>, shared_ptr<DestinationType> >
		{
			static shared_ptr<DestinationType> Do(const shared_ptr<SourceType>& source)
			{ return dynamic_pointer_cast<DestinationType>(source); }
		};


		template< typename SourceType, bool AllowNullSource = false >
		class CheckedDynamicCaster
		{
			SourceType		_source;
			ToolkitWhere	_where;

		public:
			CheckedDynamicCaster(const SourceType& source, ToolkitWhere where)
				: _source(source), _where(where)
			{ }

			template< typename TargetType >
			operator TargetType() const
			{
				TargetType target = Caster<SourceType, TargetType>::Do(_source);

				if (target || (!_source && AllowNullSource))
					return target;

				throw stingray::Detail::MakeException(InvalidCastException(_source ? TypeInfo(*_source).GetName() : "null", TypeInfo(typeid(TargetType)).GetName()), _where);
			}
		};


		template < typename T >
		CheckedDynamicCaster<T> checked_dynamic_caster(const T& source, ToolkitWhere where)
		{ return CheckedDynamicCaster<T>(source, where); }


		template < typename T >
		CheckedDynamicCaster<T, true> nullable_checked_dynamic_caster(const T& source, ToolkitWhere where)
		{ return CheckedDynamicCaster<T, true>(source, where); }
	}


#define STINGRAYKIT_CHECKED_DYNAMIC_CASTER(Expr_) stingray::Detail::checked_dynamic_caster(Expr_, STINGRAYKIT_WHERE)
#define STINGRAYKIT_NULLABLE_CHECKED_DYNAMIC_CASTER(Expr_) stingray::Detail::nullable_checked_dynamic_caster(Expr_, STINGRAYKIT_WHERE)


}


#endif
