#ifndef STINGRAY_TOOLKIT_OPTIONAL_H
#define STINGRAY_TOOLKIT_OPTIONAL_H

#include <stingray/toolkit/aligned_storage.h>
#include <stingray/toolkit/exception.h>
#include <stingray/toolkit/safe_bool.h>

namespace stingray
{


	namespace Detail
	{
		template<typename T>
		struct OptionalImplInplace
		{
		public:
			typedef T&			ParamType;
			typedef const T&	ConstParamType;
			typedef T*			PtrParamType;
			typedef const T*	ConstPtrParamType;

		private:
			StorageFor<T>	_value;
			bool			_initialized;

		public:
			OptionalImplInplace() : _initialized(false)
			{}
			ConstParamType get() const			{ return _value.Ref(); }
			ParamType get()						{ return _value.Ref(); }

			ConstPtrParamType get_ptr() const	{ return &_value.Ref(); }
			PtrParamType get_ptr()				{ return &_value.Ref(); }

			void reset()						{ Cleanup(); }
			void set(ConstParamType value)		{ Cleanup(); _value.Ctor(value); _initialized = true; }

			bool is_initialized() const			{ return _initialized; }

		private:
			void Cleanup()
			{
				if (_initialized)
				{
					_value.Dtor();
					_initialized = false;
				}
			}
		};


		template<typename T>
		struct OptionalImplRef
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
			void set(ConstParamType value)		{ _value = &value; _initialized = true; }

			bool is_initialized() const			{ return _initialized; }
		};


		template
		<
			typename T,
			bool IsRef = IsReference<T>::Value
		>
		struct OptionalImplSelector
		{
			typedef OptionalImplInplace<T> ValueT;
		};

		template<typename T>
		struct OptionalImplSelector<T, true>
		{
			typedef OptionalImplRef<T> ValueT;
		};
	}

	template<typename T>
	struct optional : public safe_bool<optional<T> >
	{
	private:
		typedef typename Detail::OptionalImplSelector<T>::ValueT	Impl;

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
		optional(ConstParamType value)				{ set(value); }
		optional(const optional& other)				{ assign(other); }

		~optional()									{ reset(); }

		optional& operator=(const NullPtrType&)		{ reset();			return *this; }
		optional& operator=(ConstParamType value)	{ set(value);		return *this; }
		optional& operator=(const optional& other)	{ assign(other);	return *this; }

		ConstParamType get() const					{ CheckInitialized(); return _impl.get(); }
		ParamType      get()						{ CheckInitialized(); return _impl.get(); }

		ConstPtrParamType get_ptr() const			{ CheckInitialized(); return _impl.get_ptr(); }
		PtrParamType get_ptr()						{ CheckInitialized(); return _impl.get_ptr(); }

		ConstPtrParamType operator->() const		{ return get_ptr(); }
		PtrParamType operator->()					{ return get_ptr(); }

		ConstParamType operator*() const			{ return get(); }
		ParamType operator*()						{ return get(); }

		void reset()								{ _impl.reset(); }
		void set(ConstParamType value)				{ _impl.set(value); }

		bool is_initialized() const					{ return _impl.is_initialized(); }
		bool boolean_test() const					{ return is_initialized(); }

		bool operator<(const optional& rhs) const
		{ return rhs.is_initialized() && (!is_initialized() || (get() < rhs.get())); }
		TOOLKIT_GENERATE_COMPARISON_OPERATORS_FROM_LESS(optional);

	private:
		void CheckInitialized() const				{ TOOLKIT_CHECK(is_initialized(), "Not initialized!"); }
		void assign(const optional& other)
		{
			if (other.is_initialized())
				set(other.get());
			else
				reset();
		}
	};

}


#endif

