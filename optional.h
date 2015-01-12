#ifndef STINGRAY_TOOLKIT_OPTIONAL_H
#define STINGRAY_TOOLKIT_OPTIONAL_H

#include <stingray/toolkit/aligned_storage.h>
#include <stingray/toolkit/exception.h>
#include <stingray/toolkit/safe_bool.h>

namespace stingray
{

	template<typename T>
	struct optional : public safe_bool<optional<T> >
	{
	private:
		StorageFor<T>	_value;
		bool			_initialized;

	public:
		typedef T&			ParamType;
		typedef const T&	ConstParamType;
		typedef T*			PtrParamType;
		typedef const T*	ConstPtrParamType;


	public:
		optional() : _initialized(false)
		{ }
		optional(const NullPtrType&) : _initialized(false)
		{ }
		optional(ConstParamType value): _initialized(false)
		{ assign(value); }
		optional(const optional& other): _initialized(false)
		{ assign(other); }

		~optional()									{ reset(); }

		optional& operator=(const NullPtrType&)		{ reset();			return *this; }
		optional& operator=(ConstParamType value)	{ assign(value);	return *this; }
		optional& operator=(const optional& other)	{ assign(other);	return *this; }

		ConstParamType get() const					{ CheckInitialized(); return _value.Ref(); }
		ParamType      get()						{ CheckInitialized(); return _value.Ref(); }

		ConstPtrParamType get_ptr() const			{ CheckInitialized(); return &_value.Ref(); }
		PtrParamType get_ptr()						{ CheckInitialized(); return &_value.Ref(); }

		ConstPtrParamType operator->() const		{ return get_ptr(); }
		PtrParamType operator->()					{ return get_ptr(); }

		ConstParamType operator*() const			{ return get(); }
		ParamType operator*()						{ return get(); }

		void reset()
		{
			if (_initialized)
			{
				_initialized = false;
				_value.Dtor();
			}
		}

		bool is_initialized() const					{ return _initialized; }
		bool boolean_test() const					{ return is_initialized(); }

		bool operator<(const optional& rhs) const
		{ return rhs.is_initialized() && (!is_initialized() || (get() < rhs.get())); }
		STINGRAYKIT_GENERATE_COMPARISON_OPERATORS_FROM_LESS(optional);

		void assign(ConstParamType value)
		{ reset(); _value.Ctor(value); _initialized = true; }

		void assign(const optional& other)
		{
			if (other.is_initialized())
				assign(other.get());
			else
				reset();
		}

	private:
		void CheckInitialized() const				{ STINGRAYKIT_CHECK(is_initialized(), "Not initialized!"); }
	};

}


#endif

