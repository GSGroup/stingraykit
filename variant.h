#ifndef STINGRAY_TOOLKIT_VARIANT_H
#define STINGRAY_TOOLKIT_VARIANT_H


#include <typeinfo>

#include <stingray/toolkit/MetaProgramming.h>
#include <stingray/toolkit/MultiStorageFor.h>
#include <stingray/toolkit/StringUtils.h>
#include <stingray/toolkit/exception.h>
#include <stingray/toolkit/fatal.h>
#include <stingray/toolkit/reference.h>
#include <stingray/toolkit/static_visitor.h>


#define STINGRAY_ASSURE_NOTHROW(ErrorMessage_, ...) \
		do { \
			try { __VA_ARGS__; } \
			catch (const std::exception& ex) { TOOLKIT_FATAL(StringBuilder() % ErrorMessage_ % ":\n" % stingray::diagnostic_information(ex)); } \
		} while (0)


namespace stingray
{

	/**
	 * @addtogroup toolkit_general_variants
	 * @{
	 */

	class bad_variant_get : public std::bad_cast
	{
	private:
		std::string		_message;
	public:
		bad_variant_get(const std::string& to, const std::string& from) : _message("Bad 'variant' get (" + to + " type requested, variant type is " + from + ")!") { }
		virtual ~bad_variant_get() throw() { }

		virtual const char* what() const throw() { return _message.c_str(); }
	};


	namespace Detail
	{

		template<typename Visitor, typename Variant, bool HasRetType = !SameType<typename Visitor::RetType, void>::Value>
		struct VariantFunctorApplier
		{
			template<int Index>
			struct ApplierHelper
			{
				static bool Call(const Visitor& v, Variant& t, optional<typename Visitor::RetType>& result)
				{
					if (t.which() != Index)
						return true;
					result = v.template Call<typename GetTypeListItem<typename Variant::TypeList, Index>::ValueT>(t);
					return false; // stop ForIf
				}
			};

			static typename Visitor::RetType Apply(const Visitor& v, Variant& var)
			{
				typedef typename Visitor::RetType RetType;
				optional<RetType> result;
				if (ForIf<GetTypeListLength<typename Variant::TypeList>::Value, ApplierHelper>::Do(v, ref(var), ref(result)))
					TOOLKIT_FATAL(StringBuilder() % "Unknown type index: " % var.which());
				return *result;
			}
		};

		template<typename Visitor, typename Variant>
		struct VariantFunctorApplier<Visitor, Variant, false>
		{
			template<int Index>
			struct ApplierHelper
			{
				static bool Call(const Visitor& v, Variant& t)
				{
					if (t.which() != Index)
						return true;
					v.template Call<typename GetTypeListItem<typename Variant::TypeList, Index>::ValueT>(t);
					return false; // stop ForIf
				}
			};

			static void Apply(const Visitor& v, Variant& var)
			{
				if (ForIf<GetTypeListLength<typename Variant::TypeList>::Value, ApplierHelper>::Do(v, ref(var)))
					TOOLKIT_FATAL(StringBuilder() % "Unknown type index: " % var.which());
			}
		};


		template <typename TypeList_>
		struct VariantBase
		{
			typedef TypeList_					TypeList;

		protected:
			typedef VariantBase<TypeList>		MyType;
			typedef MultiStorageFor<TypeList>	Storage;

			int		_type;
			Storage _storage;

		public:
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

			std::string ToString() const
			{ return ApplyVisitor(ToStringVisitor()); }

		protected:
			template<typename Visitor>
			typename Visitor::RetType ApplyFunctor(const Visitor& v)
			{ return Detail::VariantFunctorApplier<Visitor, MyType>::Apply(v, ref(*this)); }

			template<typename Visitor>
			typename Visitor::RetType ApplyFunctor(const Visitor& v) const
			{ return Detail::VariantFunctorApplier<Visitor, const MyType>::Apply(v, *this); }

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
				void Call(MyType& t) const { STINGRAY_ASSURE_NOTHROW("Destructor of variant item threw an exception: ", t._storage.template Dtor<T>()); }
			};

			void Destruct() { ApplyFunctor(DestructorFunctor()); }

			template<typename T>
			T& GetRef()				{ return _storage.template Ref<T>(); }

			template<typename T>
			const T& GetRef() const	{ return _storage.template Ref<T>(); }

			template<typename T>
			void CheckCanContain() const
			{
				CompileTimeAssert<TypeListContains<TypeList, T>::Value> ERROR__invalid_type_for_variant;
				(void)ERROR__invalid_type_for_variant;
			}

			template<typename T>
			void CheckContains() const
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

			struct ToStringVisitor : static_visitor<std::string>
			{
				template<typename T>
				std::string operator()(const T& t) const { return stingray::ToString(t); }
			};

			template < typename VariantType >
			struct LessVisitor : static_visitor<bool>
			{
			private:
				const VariantType& _rhs;

			public:
				LessVisitor(const VariantType& rhs) : _rhs(rhs)
				{}

				template<typename T>
				bool operator()(const T& t) const { return t < _rhs.template get<T>(); }
			};
		};
	}


	template <typename TypeList, bool CanBeEmpty = TypeListContains<TypeList, EmptyType>::Value>
	class variant : public Detail::VariantBase<TypeList>
	{
		typedef Detail::VariantBase<TypeList>	base;
		typedef variant<TypeList, CanBeEmpty>	MyType;
		typedef typename TypeList::ValueT		DefaultType;

	public:
		variant()
		{ AssignVal(DefaultType()); }

		template < typename T >
		variant(const T &val)
		{ AssignVal(val); }

		variant(const variant &other)
		{ Assign(other); }

		~variant()
		{ base::Destruct(); }

		variant & operator= (const variant& other)
		{
			base::Destruct();
			Assign(other);
			return *this;
		}

		template<typename T>
		variant & operator= (const T & val)
		{
			base::Destruct();
			AssignVal(val);
			return *this;
		}

		bool empty() const { return false; }

		bool operator < (const variant& other) const
		{
			typedef typename base::template LessVisitor<MyType> VisitorType;
			if (this->which() != other.which())
				return this->which() < other.which();
			return this->ApplyVisitor(VisitorType(other));
		}

		TOOLKIT_GENERATE_COMPARISON_OPERATORS_FROM_LESS(variant);

	private:
		template <typename T>
		void AssignVal(const T& val)
		{
			STINGRAY_ASSURE_NOTHROW("Copy-ctor of never-empty variant item threw an exception: ", this->_storage.template Ctor<T>(val));
			this->_type = IndexOfTypeListItem<TypeList, T>::Value;
		}

		struct CopyCtorVisitor : static_visitor<>
		{
		private:
			MyType& _target;

		public:
			CopyCtorVisitor(MyType& t) : _target(t)
			{}

			template<typename T>
			void operator()(const T& t) const { _target.AssignVal(t); }
		};

		void Assign(const MyType& other) { other.ApplyVisitor(CopyCtorVisitor(*this)); }
	};


	template <typename TypeList>
	class variant<TypeList, true> : public Detail::VariantBase<TypeList>
	{
		typedef Detail::VariantBase<TypeList>	base;
		typedef variant<TypeList, true>			MyType;
		typedef EmptyType						DefaultType;

	public:
		variant()
		{ AssignDefault(); }

		template < typename T >
		variant(const T &val)
		{ AssignVal(val); }

		variant(const variant &other)
		{ Assign(other); }

		~variant()
		{ base::Destruct(); }

		variant & operator= (const variant& other)
		{
			base::Destruct();
			Assign(other);
			return *this;
		}

		template<typename T>
		variant & operator= (const T & val)
		{
			base::Destruct();
			AssignVal(val);
			return *this;
		}

		bool empty() const { return base::which() == IndexOfTypeListItem<TypeList, EmptyType>::Value; }

		bool operator < (const variant& other) const
		{
			typedef typename base::template LessVisitor<MyType> VisitorType;
			if (this->which() != other.which())
				return this->which() < other.which();
			return this->ApplyVisitor(VisitorType(other));
		}

		TOOLKIT_GENERATE_COMPARISON_OPERATORS_FROM_LESS(variant);

	private:
		template <typename T>
		void AssignVal(const T& val)
		{
			try
			{
				this->_storage.template Ctor<T>(val);
				this->_type = IndexOfTypeListItem<TypeList, T>::Value;
			}
			catch (const std::exception& ex)
			{ AssignDefault(); throw; }
		}

		void AssignDefault()
		{
			this->_storage.template Ctor<EmptyType>();
			this->_type = IndexOfTypeListItem<TypeList, EmptyType>::Value;
		}

		struct CopyCtorVisitor : static_visitor<>
		{
		private:
			MyType& _target;

		public:
			CopyCtorVisitor(MyType& t) : _target(t)
			{}

			template<typename T>
			void operator()(const T& t) const { _target.AssignVal(t); }
		};

		void Assign(const MyType& other)
		{ other.ApplyVisitor(CopyCtorVisitor(*this)); }
	};


	template<typename T, typename TypeList>
	T* variant_get(variant<TypeList>* v)
	{ return v->template get_ptr<T>(); }

	template<typename T, typename TypeList>
	const T* variant_get(const variant<TypeList>* v)
	{ return v->template get_ptr<T>(); }

	template<typename T, typename TypeList>
	T& variant_get(variant<TypeList>& v)
	{ return v.template get<T>(); }

	template<typename T, typename TypeList>
	const T& variant_get(const variant<TypeList>& v)
	{ return v.template get<T>(); }

	template<typename Visitor, typename TypeList>
	typename Visitor::RetType apply_visitor(Visitor& visitor, variant<TypeList>& v)
	{ return v.ApplyVisitor(visitor); }

	template<typename Visitor, typename TypeList>
	typename Visitor::RetType apply_visitor(const Visitor& visitor, variant<TypeList>& v)
	{ return v.ApplyVisitor(visitor); }

	template<typename Visitor, typename TypeList>
	typename Visitor::RetType apply_visitor(Visitor& visitor, const variant<TypeList>& v)
	{ return v.ApplyVisitor(visitor); }

	template<typename Visitor, typename TypeList>
	typename Visitor::RetType apply_visitor(const Visitor& visitor, const variant<TypeList>& v)
	{ return v.ApplyVisitor(visitor); }


	/** @} */

}

#endif
