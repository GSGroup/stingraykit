#ifndef STINGRAY_TOOLKIT_VARIANT_H
#define STINGRAY_TOOLKIT_VARIANT_H


#include <stingray/toolkit/MetaProgramming.h>
#include <stingray/toolkit/any.h>


namespace stingray
{

	template < typename Typelist_ >
	class variant
	{
		template<typename T, typename SomeTypelist_> friend T* variant_get(variant<SomeTypelist_>* v);
		template<typename T, typename SomeTypelist_> friend const T* variant_get(const variant<SomeTypelist_>* v);
		template<typename T, typename SomeTypelist_> friend T& variant_get(variant<SomeTypelist_>& v);
		template<typename T, typename SomeTypelist_> friend const T& variant_get(const variant<SomeTypelist_>& v);
	
		typedef typename GetTypeListItem<Typelist_, 0>::ValueT	DefaultType;

		template < int N >
		struct WhichFunc
		{
			static bool Call(const any* data, int& result)
			{
				if (!any_cast<typename GetTypeListItem<Typelist_, N>::ValueT>(data))
					return true;

				result = N;
				return false; // break the loop
			}
		};

	private:
		any _data; // TODO: reimplement

	public:
		typedef Typelist_ typelist;

		variant()
			: _data(DefaultType())
		{ }

		template < typename T >
		variant(const T &val)
			: _data(val)
		{ CompileTimeAssert<TypeListContains<Typelist_, T>::Value> ERROR__invalid_type_for_variant; }

		//template < typename OtherTypelist > 
		//variant(const variant<OtherTypelist>& other);

		void swap(variant& other)
		{ std::swap(_data, other._data); }

		variant & operator= (const variant& other)
		{
			variant tmp(other);
			swap(tmp);
			return *this;
		}
		
		template<typename T>
		variant & operator= (const T & val)
		{
			variant tmp(val);
			swap(tmp);
			return *this;
		}

		int which() const
		{
			int result = -1;
			ForIf<GetTypeListLength<Typelist_>::Value, WhichFunc>::Do(&_data, ref(result));
			return result;
		}

		bool empty() const
		{ return _data.empty(); }

		//const std::type_info & type() const;

		//bool operator==(const variant &) const;
		//template<typename U> void operator==(const U &) const;

		//bool operator<(const variant &) const;
		//template<typename U> void operator<(const U &) const;
	};

	class bad_variant_get : public std::bad_cast
	{
	private:
		std::string		_message;
	public:
		bad_variant_get(const std::string& to) : _message("Bad 'variant' get (" + to + " type requested)!") { }
		virtual ~bad_variant_get() throw() { }

		virtual const char* what() const throw() { return _message.c_str(); }
	};


	template<typename T, typename Typelist_> 
	T* variant_get(variant<Typelist_>* v)
	{
		CompileTimeAssert<TypeListContains<Typelist_, T>::Value> ERROR__invalid_type_for_variant;
		try { return v ? any_cast<T>(&v->_data) : NULL; }
		catch (const bad_any_cast&) { throw(bad_variant_get(Demangle(typeid(T).name()))); }
	}

	template<typename T, typename Typelist_> 
	const T* variant_get(const variant<Typelist_>* v)
	{
		CompileTimeAssert<TypeListContains<Typelist_, T>::Value> ERROR__invalid_type_for_variant;
		try { return v ? any_cast<T>(&v->_data) : NULL; }
		catch (const bad_any_cast&) { throw(bad_variant_get(Demangle(typeid(T).name()))); }
	}

	template<typename T, typename Typelist_> 
	T& variant_get(variant<Typelist_>& v)
	{
		CompileTimeAssert<TypeListContains<Typelist_, T>::Value> ERROR__invalid_type_for_variant;
		try { return any_cast<T>(v._data); }
		catch (const bad_any_cast&) { throw(bad_variant_get(Demangle(typeid(T).name()))); }
	}

	template<typename T, typename Typelist_> 
	const T& variant_get(const variant<Typelist_>& v)
	{
		CompileTimeAssert<TypeListContains<Typelist_, T>::Value> ERROR__invalid_type_for_variant;
		try { return any_cast<T>(v._data); }
		catch (const bad_any_cast&) { throw(bad_variant_get(Demangle(typeid(T).name()))); }
	}

}

#endif
