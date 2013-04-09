#ifndef STINGRAY_TOOLKIT_DYNAMIC_CASTER_H
#define STINGRAY_TOOLKIT_DYNAMIC_CASTER_H


#include <stingray/toolkit/MetaProgramming.h>
#include <stingray/toolkit/shared_ptr.h>
#include <stingray/toolkit/exception.h>
#include <stingray/toolkit/Dummy.h>


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

		//operator const DynamicPointerCaster& () const { return *this } // workaround for an old GCC bug

		template < typename U > operator U* () const
		{
			return dynamic_cast<U*>(_ptr);
		}

		operator const DynamicCaster&() const { return *this; }

		template < typename U > operator U& () const
		{
			CompileTimeAssert<!SameType<typename Deconst<U>::ValueT, DynamicCaster>::Value> ERROR__old_gcc_bug;
			CompileTimeAssert<!IsPointer<U>::Value> ERROR__wtf;
			(void)ERROR__old_gcc_bug;
			(void)ERROR__wtf;
			U* result_ptr = dynamic_cast<U*>(_ptr);
			if (!result_ptr)
				TOOLKIT_THROW(std::bad_cast());
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

		//operator const DynamicPointerCaster& () const { return *this } // workaround for an old GCC bug

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


	template < typename T > DynamicCaster<typename EnableIf<!Is1ParamTemplate<shared_ptr, T>::Value, T>::ValueT>
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
			const char*		_file;
			size_t			_line;
			const char*		_functionName;

		public:
			CheckedDynamicCaster(const SourceType& source, const char* file, size_t line, const char* functionName)
				: _source(source), _file(file), _line(line), _functionName(functionName)
			{ }

			template< typename TargetType >
			operator TargetType() const
			{
				TargetType target = Caster<SourceType, TargetType>::Do(_source);

				if ((!AllowNullSource || _source) && !target)
					throw stingray::Detail::MakeException(InvalidCastException(Demangle(typeid(SourceType).name()), Demangle(typeid(TargetType).name())), _file, _line, _functionName);

				return target;
			}
		};


		template < typename T >
		CheckedDynamicCaster<T> checked_dynamic_caster(const T& source, const char* file, int line, const char* functionName)
		{ return CheckedDynamicCaster<T>(source, file, line, functionName); }


		template < typename T >
		CheckedDynamicCaster<T, true> nullable_checked_dynamic_caster(const T& source, const char* file, int line, const char* functionName)
		{ return CheckedDynamicCaster<T, true>(source, file, line, functionName); }
	}


#define TOOLKIT_CHECKED_DYNAMIC_CASTER(Expr_) stingray::Detail::checked_dynamic_caster(Expr_, __FILE__, __LINE__, TOOLKIT_FUNCTION)
#define TOOLKIT_NULLABLE_CHECKED_DYNAMIC_CASTER(Expr_) stingray::Detail::nullable_checked_dynamic_caster(Expr_, __FILE__, __LINE__, TOOLKIT_FUNCTION)


}


#endif
