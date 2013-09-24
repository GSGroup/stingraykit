#ifndef STINGRAY_TOOLKIT_VARIANT_H
#define STINGRAY_TOOLKIT_VARIANT_H


#include <typeinfo>

#include <stingray/toolkit/MetaProgramming.h>
#include <stingray/toolkit/MultiStorageFor.h>
#include <stingray/toolkit/any.h>
#include <stingray/toolkit/exception.h>
#include <stingray/toolkit/static_visitor.h>


#define STINGRAY_ASSURE_NOTHROW(ErrorMessage_, ...) \
		do { \
			try { __VA_ARGS__; } \
			catch (const std::exception& ex) { TOOLKIT_FATAL(StringBuilder() % ErrorMessage_ % ":\n" % stingray::diagnostic_information(ex)); } \
		} while (0)


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

		template < int N >
		struct GetTypeInfoFunc
		{
			static bool Call(const any* data, const std::type_info*& result)
			{
				typedef typename GetTypeListItem<Typelist_, N>::ValueT	Type;
				if (!any_cast<Type>(data))
					return true;

				result = &typeid(Type);
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

		const std::type_info & type() const
		{
			const std::type_info* result = NULL;
			ForIf<GetTypeListLength<Typelist_>::Value, GetTypeInfoFunc>::Do(&_data, ref(result));
			return *TOOLKIT_REQUIRE_NOT_NULL(result);
		}

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
		bad_variant_get(const std::string& to, const std::string& from) : _message("Bad 'variant' get (" + to + " type requested, variant type is " + from + ")!") { }
		virtual ~bad_variant_get() throw() { }

		virtual const char* what() const throw() { return _message.c_str(); }
	};


	template<typename T, typename Typelist_>
	T* variant_get(variant<Typelist_>* v)
	{
		CompileTimeAssert<TypeListContains<Typelist_, T>::Value> ERROR__invalid_type_for_variant;
		try { return v ? any_cast<T>(&v->_data) : NULL; }
		catch (const bad_any_cast&) { throw(bad_variant_get(Demangle(typeid(T).name()), Demangle(v->type().name()))); }
	}

	template<typename T, typename Typelist_>
	const T* variant_get(const variant<Typelist_>* v)
	{
		CompileTimeAssert<TypeListContains<Typelist_, T>::Value> ERROR__invalid_type_for_variant;
		try { return v ? any_cast<T>(&v->_data) : NULL; }
		catch (const bad_any_cast&) { throw(bad_variant_get(Demangle(typeid(T).name()), Demangle(v->type().name()))); }
	}

	template<typename T, typename Typelist_>
	T& variant_get(variant<Typelist_>& v)
	{
		CompileTimeAssert<TypeListContains<Typelist_, T>::Value> ERROR__invalid_type_for_variant;
		try { return any_cast<T>(v._data); }
		catch (const bad_any_cast&) { throw(bad_variant_get(Demangle(typeid(T).name()), Demangle(v.type().name()))); }
	}

	template<typename T, typename Typelist_>
	const T& variant_get(const variant<Typelist_>& v)
	{
		CompileTimeAssert<TypeListContains<Typelist_, T>::Value> ERROR__invalid_type_for_variant;
		try { return any_cast<T>(v._data); }
		catch (const bad_any_cast&) { throw(bad_variant_get(Demangle(typeid(T).name()), Demangle(v.type().name()))); }
	}


	template <typename TypeList>
	class VariantNothrowImpl
	{
		typedef VariantNothrowImpl<TypeList> MyType;

		typedef MultiStorageFor<TypeList> Storage;
		typedef typename TypeList::ValueT DefaultType;

		int		_type;
		Storage _storage;

	public:
		VariantNothrowImpl()
		{
			_storage.template Ctor<DefaultType>();
			_type = 0;
		}

		template < typename T >
		VariantNothrowImpl(const T &val)
		{ Assign(val); }

		VariantNothrowImpl(const VariantNothrowImpl &other)
		{ Assign(other); }

		//template < typename OtherTypelist >
		//VariantNothrowImpl(const VariantNothrowImpl<OtherTypelist>& other);

		VariantNothrowImpl & operator= (const VariantNothrowImpl& other)
		{
			Destruct();
			Assign(other);
			return *this;
		}

		template<typename T>
		VariantNothrowImpl & operator= (const T & val)
		{
			Destruct();
			Assign(val);
			return *this;
		}

		int which() const
		{ return _type; }

		const std::type_info& type() const
		{ return *TOOLKIT_REQUIRE_NOT_NULL(ApplyVisitor(GetTypeInfoVisitor())); }

		template<typename Visitor>
		typename Visitor::RetType ApplyVisitor(Visitor& v)
		{
			VisitorApplier<Visitor> applier(v);
			return ApplyFunctor(applier);
		}

		template<typename Visitor>
		typename Visitor::RetType ApplyVisitor(const Visitor& v)
		{
			VisitorApplier<const Visitor> applier(v);
			return ApplyFunctor(applier);
		}

		template<typename Visitor>
		typename Visitor::RetType ApplyVisitor(Visitor& v) const
		{
			VisitorApplier<Visitor> applier(v);
			return ApplyFunctor(applier);
		}

		template<typename Visitor>
		typename Visitor::RetType ApplyVisitor(const Visitor& v) const
		{
			VisitorApplier<const Visitor> applier(v);
			return ApplyFunctor(applier);
		}

		template<typename T>
		T* get_ptr()
		{
			CheckCanContain<T>();
			if (_type != IndexOfTypeListItem<TypeList, T>::Value)
				return null;
			return &GetRef<T>();
		}

		template<typename T>
		const T* get_ptr() const
		{
			CheckCanContain<T>();
			if (_type != IndexOfTypeListItem<TypeList, T>::Value)
				return null;
			return &GetRef<T>();
		}

		template<typename T>
		T& get()
		{
			CheckContains<T>();
			return GetRef<T>();
		}

		template<typename T>
		const T& get() const
		{
			CheckContains<T>();
			return GetRef<T>();
		}

	private:
		template<int Index>
		struct ApplierHelper
		{
			template<typename Visitor>
			static bool Call(const Visitor& v, MyType& t, typename Visitor::RetType& result)
			{
				if (t._type != Index)
					return true;
				result = v.template Call<typename GetTypeListItem<TypeList, Index>::ValueT>(t);
				return false; // stop ForIf
			}

			template<typename Visitor>
			static bool Call(const Visitor& v, const MyType& t, typename Visitor::RetType& result)
			{
				if (t._type != Index)
					return true;
				result = v.template Call<typename GetTypeListItem<TypeList, Index>::ValueT>(t);
				return false; // stop ForIf
			}
		};

		template<typename Visitor>
		typename Visitor::RetType ApplyFunctor(const Visitor& v)
		{
			typedef typename Visitor::RetType RetType;
			RetType result = RetType();
			if (ForIf<GetTypeListLength<TypeList>::Value, ApplierHelper>::Do(v, ref(*this), ref(result)))
				TOOLKIT_FATAL(StringBuilder() % "Unknown type index: " % _type);
			return result;
		}

		template<typename Visitor>
		typename Visitor::RetType ApplyFunctor(const Visitor& v) const
		{
			typedef typename Visitor::RetType RetType;
			RetType result = RetType();
			if (ForIf<GetTypeListLength<TypeList>::Value, ApplierHelper>::Do(v, ref(*this), ref(result)))
				TOOLKIT_FATAL(StringBuilder() % "Unknown type index: " % _type);
			return result;
		}

		template<typename Visitor>
		struct VisitorApplier : static_visitor<typename Visitor::RetType>
		{
		private:
			Visitor& _visitor;

		public:
			VisitorApplier(Visitor& v) : _visitor(v)
			{}

			template<typename T>
			typename Visitor::RetType Call(MyType& t) const			{ return _visitor(t.GetRef<T>()); }

			template<typename T>
			typename Visitor::RetType Call(const MyType& t) const	{ return _visitor(t.GetRef<T>()); }
		};

		struct DestructorFunctor : static_visitor<>
		{
			template<typename T>
			void Call(MyType& t) const { STINGRAY_ASSURE_NOTHROW(t._storage.template Dtor<T>()); }
		};

		void Destruct() { ApplyFunctor(DestructorFunctor()); }

		struct CopyCtorVisitor : static_visitor<>
		{
		private:
			MyType& _target;

		public:
			CopyCtorVisitor(MyType& t) : _target(t)
			{}

			template<typename T>
			void operator()(const T& t) const { STINGRAY_ASSURE_NOTHROW(_target._storage.template Ctor<T>(t)); }
		};

		void Assign(const MyType& other) { other.ApplyVisitor(CopyCtorVisitor(*this)); }

		template <typename T>
		void Assign(const T& val)
		{
			_storage.template Ctor<T>(val);
			_type = IndexOfTypeListItem<TypeList, T>::Value;
		}

		template<typename T>
		T& GetRef()				{ return _storage.template Ref<T>(); }

		template<typename T>
		const T& GetRef() const	{ return _storage.template Ref<T>(); }

		template<typename T>
		void CheckCanContain()
		{
			CompileTimeAssert<TypeListContains<TypeList, T>::Value> ERROR__invalid_type_for_variant;
			(void)ERROR__invalid_type_for_variant;
		}

		template<typename T>
		void CheckContains()
		{
			CheckCanContain<T>();
			if (_type != IndexOfTypeListItem<TypeList, T>::Value)
				TOOLKIT_THROW(bad_variant_get(Demangle(typeid(T).name()), Demangle(type().name())));
		}

		struct GetTypeInfoVisitor : static_visitor<const std::type_info*>
		{
			template<typename T>
			const std::type_info* operator()(const T& t) const { return &typeid(T); }
		};
	};


	template<typename T, typename TypeList>
	T* variant_get(VariantNothrowImpl<TypeList>* v)
	{ return v->template get_ptr<T>(); }

	template<typename T, typename TypeList>
	const T* variant_get(const VariantNothrowImpl<TypeList>* v)
	{ return v->template get_ptr<T>(); }

	template<typename T, typename TypeList>
	T& variant_get(VariantNothrowImpl<TypeList>& v)
	{ return v.template get<T>(); }

	template<typename T, typename TypeList>
	const T& variant_get(const VariantNothrowImpl<TypeList>& v)
	{ return v.template get<T>(); }


}

#endif
