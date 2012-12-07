#ifndef STINGRAY_TOOLKIT_OPTIONAL_H
#define STINGRAY_TOOLKIT_OPTIONAL_H

#include <stingray/toolkit/safe_bool.h>
#include <stingray/toolkit/unique_ptr.h>

namespace stingray {


	namespace Detail
	{
		template<typename T, bool IsRef = IsReference<T>::Value >
		struct OptionalHelper
		{
			typedef T									ParamType;
			typedef T									StoredType;

			static const StoredType& ToStored(const ParamType& val)	{ return val; }
			static ParamType& ToParam(StoredType& val)				{ return val; }
			static const ParamType& ToParam(const StoredType& val)	{ return val; }
		};

		template<typename T>
		struct OptionalHelper<T, true>
		{
			typedef typename Dereference<T>::ValueT		ParamType;
			typedef typename Dereference<T>::ValueT*	StoredType;

			static StoredType ToStored(const ParamType& val)		{ return &val; }
			static ParamType& ToParam(StoredType val)				{ return *val; }
		};
	}

	template<typename T>
	struct optional : public safe_bool<optional<T> >
	{
	private:
		typedef Detail::OptionalHelper<T>	Helper;
		typedef typename Helper::StoredType	StoredType;

	public:
		typedef typename Helper::ParamType	ParamType;

	private:
		unique_ptr<StoredType>				_value;

	public:
		optional()
		{}

		optional(const NullPtrType&)
		{}

		optional(const ParamType& value) : _value(new StoredType(Helper::ToStored(value)))
		{}

		optional(const optional& other)
		{
			if (other._value)
				_value.reset(new StoredType(*other._value));
		}

		template<class U>
		explicit optional(const optional<U>& other)
		{
			if (other._value)
				_value.reset(new StoredType(*other._value));
		}


		optional& operator=(const NullPtrType&)		{ reset();								return *this; }
		optional& operator=(const ParamType& value)	{ _value.reset(new StoredType(value));	return *this; }

		optional& operator=(const optional& other)
		{
			if (other._value)
				_value.reset(new StoredType(*other._value));
			else
				_value.reset();
			return *this;
		}

		template<class U>
		optional& operator=(const optional<U>& other)
		{
			if (other._value)
				_value.reset(new StoredType(*other._value));
			else
				_value.reset();
		}


	    const ParamType& get() const		{ return Helper::ToParam(*TOOLKIT_REQUIRE_NOT_NULL(_value)); }
    	ParamType&       get()				{ return Helper::ToParam(*TOOLKIT_REQUIRE_NOT_NULL(_value)); }

		const ParamType* get_ptr() const	{ return &get(); }
		ParamType* get_ptr()				{ return &get(); }

		const ParamType* operator->() const	{ return get_ptr(); }
		ParamType* operator->()				{ return get_ptr(); }

		const ParamType& operator*() const	{ return get(); }
		ParamType& operator*()				{ return get(); }

		void reset()						{ _value.reset(); }
		void reset(const ParamType& value)	{ _value.reset(new StoredType(Helper::ToStored(value))); }

		bool is_initialized() const			{ return _value; }
		bool boolean_test() const			{ return _value; }

		bool operator<(const optional& rhs) const
		{ return rhs.is_initialized() && (!is_initialized() || (get() < rhs.get())); }
		TOOLKIT_GENERATE_COMPARISON_OPERATORS_FROM_LESS(optional);
	};
	TOOLKIT_GENERATE_FREE_COMPARISON_OPERATORS_FOR_TEMPLATE_CLASS(template<typename T>, optional<T>);

}


#endif

