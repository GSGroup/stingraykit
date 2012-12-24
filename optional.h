#ifndef STINGRAY_TOOLKIT_OPTIONAL_H
#define STINGRAY_TOOLKIT_OPTIONAL_H

#include <stingray/toolkit/safe_bool.h>
#include <stingray/toolkit/unique_ptr.h>

namespace stingray
{


	namespace Detail
	{
		template<typename T, bool StoreByValue = IsBuiltinType<T>::Value || IsEnumClass<T>::Value || IsPointer<T>::Value || Is1ParamTemplate<shared_ptr, T>::Value, bool IsRef = IsReference<T>::Value>
		struct OptionalImpl
		{
		public:
			typedef T&			ParamType;
			typedef const T&	ConstParamType;
			typedef T*			PtrParamType;
			typedef const T*	ConstPtrParamType;

		private:
			unique_ptr<T>	_value;

		public:
			ConstParamType get() const			{ return *_value; }
			ParamType get()						{ return *_value; }

			ConstPtrParamType get_ptr() const	{ return _value.get(); }
			PtrParamType get_ptr()				{ return _value.get(); }

			void reset()						{ _value.reset(); }
			void reset(ConstParamType value)	{ _value.reset(new T(value)); }

			bool is_initialized() const			{ return _value; }
		};


		template<typename T>
		struct OptionalImpl<T, true, false>
		{
		public:
			typedef T&			ParamType;
			typedef const T&	ConstParamType;
			typedef T*			PtrParamType;
			typedef const T*	ConstPtrParamType;

		private:
			T		_value;
			bool	_initialized;

		public:
			ConstParamType get() const			{ return _value; }
			ParamType get()						{ return _value; }

			ConstPtrParamType get_ptr() const	{ return &_value; }
			PtrParamType get_ptr()				{ return &_value; }

			void reset()						{ _value = T(); _initialized = false; }
			void reset(ConstParamType value)	{ _value = value; _initialized = true; }

			bool is_initialized() const			{ return _initialized; }
		};


		template<typename T>
		struct OptionalImpl<T, false, true>
		{
		public:
			typedef typename Dereference<T>::ValueT RawT;
			typedef RawT&	ParamType;
			typedef RawT&	ConstParamType;
			typedef RawT*	PtrParamType;
			typedef RawT*	ConstPtrParamType;

		private:
			T*		_value;
			bool	_initialized;

		public:
			ConstParamType get() const			{ return *_value; }
			ConstPtrParamType get_ptr() const	{ return _value; }

			void reset()						{ _value = null; _initialized = false; }
			void reset(ConstParamType value)	{ _value = &value; _initialized = true; }

			bool is_initialized() const			{ return _initialized; }
		};
	}

	template<typename T>
	struct optional : public safe_bool<optional<T> >
	{
	private:
		typedef Detail::OptionalImpl<T>	Impl;

	public:
		typedef typename Impl::ParamType			ParamType;
		typedef typename Impl::ConstParamType		ConstParamType;
		typedef typename Impl::PtrParamType			PtrParamType;
		typedef typename Impl::ConstPtrParamType	ConstPtrParamType;

	private:
		Impl	_impl;

	public:
		optional()									{ reset(); }
		optional(const NullPtrType&)				{ reset(); }
		optional(ConstParamType value)				{ reset(value); }
		optional(const optional& other)				{ reset(other); }

		optional& operator=(const NullPtrType&)		{ reset();		return *this; }
		optional& operator=(ConstParamType value)	{ reset(value);	return *this; }
		optional& operator=(const optional& other)	{ reset(other);	return *this; }

		ConstParamType get() const					{ CheckInitialized(); return _impl.get(); }
		ParamType       get()						{ CheckInitialized(); return _impl.get(); }

		ConstPtrParamType get_ptr() const			{ CheckInitialized(); return _impl.get_ptr(); }
		PtrParamType get_ptr()						{ CheckInitialized(); return _impl.get_ptr(); }

		ConstPtrParamType operator->() const		{ return get_ptr(); }
		PtrParamType operator->()					{ return get_ptr(); }

		ConstParamType operator*() const			{ return get(); }
		ParamType operator*()						{ return get(); }

		void reset()								{ _impl.reset(); }
		void reset(ConstParamType value)			{ _impl.reset(value); }

		bool is_initialized() const					{ return _impl.is_initialized(); }
		bool boolean_test() const					{ return is_initialized(); }

		bool operator<(const optional& rhs) const
		{ return rhs.is_initialized() && (!is_initialized() || (get() < rhs.get())); }
		TOOLKIT_GENERATE_COMPARISON_OPERATORS_FROM_LESS(optional);

	private:
		void CheckInitialized() const				{ TOOLKIT_CHECK(is_initialized(), "Not initialized!"); }
		void reset(const optional& other)
		{
			if (other.is_initialized())
				reset(other.get());
			else
				reset();
		}
	};
	TOOLKIT_GENERATE_FREE_COMPARISON_OPERATORS_FOR_TEMPLATE_CLASS(template<typename T>, optional<T>);

}


#endif

