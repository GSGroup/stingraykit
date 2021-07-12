#ifndef STINGRAYKIT_VARIANT_H
#define STINGRAYKIT_VARIANT_H

// Copyright (c) 2011 - 2019, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/MultiStorageFor.h>
#include <stingraykit/string/ToString.h>

#include <typeinfo>

#define STINGRAYKIT_ASSURE_NOTHROW(ErrorMessage_, ...) \
		do { \
			try { __VA_ARGS__; } \
			catch (const std::exception& ex) { STINGRAYKIT_FATAL(StringBuilder() % ErrorMessage_ % ":\n" % ex); } \
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
		bad_variant_get(const std::string& to, const std::string& from) : _message(StringBuilder() % "Bad 'variant' get (" % to % " type requested, variant type is " % from % ")!") { }
		virtual ~bad_variant_get() noexcept { }

		virtual const char* what() const noexcept { return _message.c_str(); }
	};


	template < typename ResultType = void >
	struct static_visitor : public function_info<ResultType, UnspecifiedParamTypes>
	{ };


	namespace Detail
	{
		template < typename Base >
		struct InheritanceTester
		{
			template < typename Derived >
			struct Predicate : IsInherited<Derived, Base> { };
		};

		template < typename TypeList, typename Type >
		struct TypeListFilterInheritedTypes
		{
			typedef typename TypeListCopyIf<TypeList, InheritanceTester<Type>::template Predicate>::ValueT ValueT;

			static const bool IsEmpty = GetTypeListLength<ValueT>::Value == 0;
		};


		template < typename Visitor, typename Variant, bool HasRetType = !IsSame<typename Visitor::RetType, void>::Value >
		struct VariantFunctorApplier
		{
			template < size_t Index >
			struct ApplierHelper
			{
				static bool Call(const Visitor& v, Variant& t, optional<typename Visitor::RetType>& result)
				{
					if (t.which() != Index)
						return true;
					result = v.template Call<typename GetTypeListItem<typename Variant::Types, Index>::ValueT>(t);
					return false; // stop ForIf
				}
			};

			static typename Visitor::RetType Apply(const Visitor& v, Variant& var)
			{
				typedef typename Visitor::RetType RetType;
				optional<RetType> result;
				if (ForIf<GetTypeListLength<typename Variant::Types>::Value, ApplierHelper>::Do(v, wrap_ref(var), wrap_ref(result)))
					STINGRAYKIT_FATAL(StringBuilder() % "Unknown type index: " % var.which());
				return *result;
			}
		};

		template < typename Visitor, typename Variant >
		struct VariantFunctorApplier<Visitor, Variant, false>
		{
			template < size_t Index >
			struct ApplierHelper
			{
				static bool Call(const Visitor& v, Variant& t)
				{
					if (t.which() != Index)
						return true;
					v.template Call<typename GetTypeListItem<typename Variant::Types, Index>::ValueT>(t);
					return false; // stop ForIf
				}
			};

			static void Apply(const Visitor& v, Variant& var)
			{
				if (ForIf<GetTypeListLength<typename Variant::Types>::Value, ApplierHelper>::Do(v, wrap_ref(var)))
					STINGRAYKIT_FATAL(StringBuilder() % "Unknown type index: " % var.which());
			}
		};


		template < typename TypeList_ >
		struct VariantBase
		{
			typedef TypeList_					Types;

		protected:
			typedef VariantBase<Types>			MyType;
			typedef MultiStorageFor<Types>		Storage;

			size_t	_type;
			Storage _storage;

		public:
			size_t which() const
			{ return _type; }

			const std::type_info& type() const
			{ return *STINGRAYKIT_REQUIRE_NOT_NULL(ApplyVisitor(GetTypeInfoVisitor())); }

			template < typename Visitor >
			typename Visitor::RetType ApplyVisitor(Visitor& v)
			{
				VisitorApplier<Visitor> applier(v);
				return ApplyFunctor(applier);
			}

			template < typename Visitor >
			typename Visitor::RetType ApplyVisitor(const Visitor& v)
			{
				VisitorApplier<const Visitor> applier(v);
				return ApplyFunctor(applier);
			}

			template < typename Visitor >
			typename Visitor::RetType ApplyVisitor(Visitor& v) const
			{
				VisitorApplier<Visitor> applier(v);
				return ApplyFunctor(applier);
			}

			template < typename Visitor >
			typename Visitor::RetType ApplyVisitor(const Visitor& v) const
			{
				VisitorApplier<const Visitor> applier(v);
				return ApplyFunctor(applier);
			}

			template < typename T >
			T* get_ptr()
			{
				CheckCanContainInheritedType<T>();
				return GetPtr<T>();
			}

			template < typename T >
			const T* get_ptr() const
			{
				CheckCanContainInheritedType<T>();
				return GetPtr<T>();
			}

			template < typename T >
			T& get()
			{
				CheckCanContainInheritedType<T>();
				T* value = GetPtr<T>();
				if (!value)
					ThrowBadVariantGet<T>();
				return *value;
			}

			template < typename T >
			const T& get() const
			{
				CheckCanContainInheritedType<T>();
				const T* value = GetPtr<T>();
				if (!value)
					ThrowBadVariantGet<T>();
				return *value;
			}

			template < typename T >
			bool contains() const
			{
				CheckCanContainInheritedType<T>();
				return GetPtr<T>();
			}

		protected:
			template < typename Visitor >
			typename Visitor::RetType ApplyFunctor(const Visitor& v)
			{ return Detail::VariantFunctorApplier<Visitor, MyType>::Apply(v, wrap_ref(*this)); }

			template < typename Visitor >
			typename Visitor::RetType ApplyFunctor(const Visitor& v) const
			{ return Detail::VariantFunctorApplier<Visitor, const MyType>::Apply(v, *this); }

			template < typename Visitor >
			struct VisitorApplier : static_visitor<typename Visitor::RetType>
			{
			private:
				Visitor& _visitor;

			public:
				VisitorApplier(Visitor& v) : _visitor(v)
				{ }

				template < typename T >
				typename Visitor::RetType Call(MyType& t) const			{ return _visitor(t._storage.template Ref<T>()); }

				template < typename T >
				typename Visitor::RetType Call(const MyType& t) const	{ return _visitor(t._storage.template Ref<T>()); }
			};

			struct DestructorFunctor : static_visitor<>
			{
				template < typename T >
				void Call(MyType& t) const { STINGRAYKIT_ASSURE_NOTHROW("Destructor of variant item threw an exception: ", t._storage.template Dtor<T>()); }
			};

			template < typename DesiredType >
			struct GetPointerVisitor : static_visitor<DesiredType*>
			{
				template < typename Type >
				typename EnableIf<IsInherited<Type, DesiredType>::Value, DesiredType*>::ValueT operator()(Type& value)
				{ return &value; }

				template < typename Type >
				typename EnableIf<!IsInherited<Type, DesiredType>::Value, DesiredType*>::ValueT operator()(Type& value)
				{ return NULL; }
			};

			void Destruct() { ApplyFunctor(DestructorFunctor()); }

			template < typename T >
			T* GetPtr()
			{
				GetPointerVisitor<T> v;
				return ApplyVisitor(v);
			}

			template < typename T >
			const T* GetPtr() const
			{
				GetPointerVisitor<const T> v;
				return ApplyVisitor(v);
			}

			template < typename T >
			void CheckCanContain() const
			{
				static_assert(TypeListContains<Types, T>::Value, "Invalid type for variant");
			}

			template < typename T >
			void CheckCanContainInheritedType() const
			{
				typedef typename Detail::TypeListFilterInheritedTypes<Types, T> InheritedTypes;
				static_assert(!InheritedTypes::IsEmpty, "Invalid type for variant");
			}

			template < typename T >
			void ThrowBadVariantGet() const
			{ STINGRAYKIT_THROW(bad_variant_get(Demangle(typeid(T).name()), Demangle(type().name()))); }

			struct GetTypeInfoVisitor : static_visitor<const std::type_info*>
			{
				template < typename T >
				const std::type_info* operator()(const T& t) const { return &typeid(T); }
			};

			template < typename VariantType, template <typename> class ComparerType >
			struct ComparerVisitor : static_visitor<bool>
			{
			private:
				const VariantType&	_rhs;

			public:
				ComparerVisitor(const VariantType& rhs) : _rhs(rhs)
				{ }

				template < typename T >
				bool operator () (const T& t) const { return ComparerType<T>()(t, _rhs.template get<T>()); }
			};
		};


		template < typename TypeList_, bool StringableTypeList = TypeListAllOf<TypeList_, IsStringRepresentable>::Value >
		struct StringRepresentableVariant : public VariantBase<TypeList_>
		{
		private:
			struct ToStringVisitor : static_visitor<std::string>
			{
				template < typename T >
				std::string operator () (const T& t) const { return stingray::ToString(t); }
			};

		public:
			std::string ToString() const
			{ return this->ApplyVisitor(ToStringVisitor()); }
		};

		template < typename TypeList_ >
		struct StringRepresentableVariant<TypeList_, false> : public VariantBase<TypeList_>
		{ };
	}


	template < typename TypeList, bool CanBeEmpty = TypeListContains<TypeList, EmptyType>::Value >
	class variant : public Detail::StringRepresentableVariant<TypeList>
	{
		typedef Detail::VariantBase<TypeList>	base;
		typedef variant<TypeList, CanBeEmpty>	MyType;
		typedef typename TypeList::ValueT		DefaultType;

	public:
		variant()
		{ AssignVal(DefaultType()); }

		template < typename T >
		variant(T&& val, typename EnableIf<TypeListContains<TypeList, typename Decay<T>::ValueT>::Value, Dummy>::ValueT* = 0)
		{ AssignVal(std::forward<T>(val)); }

		variant(const variant& other)
		{ Assign(other); }

		variant(variant&& other)
		{ Assign(std::move(other)); }

		~variant()
		{ base::Destruct(); }

		void assign(const variant& other)
		{
			base::Destruct();
			Assign(other);
		}

		void assign(variant&& other)
		{
			base::Destruct();
			Assign(std::move(other));
		}

		template < typename T >
		void assign(T&& val, typename EnableIf<TypeListContains<TypeList, typename Decay<T>::ValueT>::Value, Dummy>::ValueT* = 0)
		{
			base::Destruct();
			AssignVal(std::forward<T>(val));
		}

		variant& operator = (const variant& other)
		{
			assign(other);
			return *this;
		}

		variant& operator = (variant&& other)
		{
			assign(std::move(other));
			return *this;
		}

		template < typename T >
		typename EnableIf<TypeListContains<TypeList, typename Decay<T>::ValueT>::Value, variant&>::ValueT operator = (T&& val)
		{
			assign(std::forward<T>(val));
			return *this;
		}

		bool empty() const { return false; }

		bool operator == (const variant& other) const
		{
			typedef typename base::template ComparerVisitor<MyType, std::equal_to> VisitorType;
			if (this->which() != other.which())
				return false;
			return this->ApplyVisitor(VisitorType(other));
		}

		STINGRAYKIT_GENERATE_EQUALITY_OPERATORS_FROM_EQUAL(variant);

		bool operator < (const variant& other) const
		{
			typedef typename base::template ComparerVisitor<MyType, std::less> VisitorType;
			if (this->which() != other.which())
				return this->which() < other.which();
			return this->ApplyVisitor(VisitorType(other));
		}

		STINGRAYKIT_GENERATE_RELATIONAL_OPERATORS_FROM_LESS(variant);

		template < typename T, typename... Us >
		void emplace(Us&&... args)
		{
			base::Destruct();
			STINGRAYKIT_ASSURE_NOTHROW("Ctor of never-empty variant item threw an exception", this->_storage.template Ctor<T>(std::forward<Us>(args)...));
			this->_type = IndexOfTypeListItem<TypeList, T>::Value;
		}

	private:
		template < typename T >
		void AssignVal(T&& val, typename EnableIf<TypeListContains<TypeList, typename Decay<T>::ValueT>::Value, Dummy>::ValueT* = 0)
		{
			STINGRAYKIT_ASSURE_NOTHROW("Copy-ctor of never-empty variant item threw an exception: ", this->_storage.template Ctor<typename Decay<T>::ValueT>(std::forward<T>(val)));
			this->_type = IndexOfTypeListItem<TypeList, typename Decay<T>::ValueT>::Value;
		}

		struct CopyCtorVisitor : static_visitor<>
		{
		private:
			MyType& _target;

		public:
			CopyCtorVisitor(MyType& t) : _target(t)
			{ }

			template < typename T >
			void operator () (const T& t) const { _target.AssignVal(t); }
		};

		void Assign(const MyType& other)
		{ other.ApplyVisitor(CopyCtorVisitor(*this)); }

		struct MoveCtorVisitor : static_visitor<>
		{
		private:
			MyType& _target;

		public:
			MoveCtorVisitor(MyType& t) : _target(t)
			{ }

			template < typename T >
			void operator () (T& t) const
			{
				static_assert(!IsConst<T>::Value, "Move won't work");
				_target.AssignVal(std::move(t));
			}
		};

		void Assign(MyType&& other)
		{ other.ApplyVisitor(MoveCtorVisitor(*this)); }
	};


	template < typename TypeList >
	class variant<TypeList, true> : public Detail::StringRepresentableVariant<TypeList>
	{
		typedef Detail::VariantBase<TypeList>	base;
		typedef variant<TypeList, true>			MyType;
		typedef EmptyType						DefaultType;

	public:
		variant()
		{ AssignDefault(); }

		template < typename T >
		variant(T&& val, typename EnableIf<TypeListContains<TypeList, typename Decay<T>::ValueT>::Value, Dummy>::ValueT* = 0)
		{ AssignVal(std::forward<T>(val)); }

		variant(const variant& other)
		{ Assign(other); }

		variant(variant&& other)
		{ Assign(std::move(other)); }

		~variant()
		{ base::Destruct(); }

		void assign(const variant& other)
		{
			base::Destruct();
			Assign(other);
		}

		void assign(variant&& other)
		{
			base::Destruct();
			Assign(std::move(other));
		}

		template < typename T >
		void assign(T&& val, typename EnableIf<TypeListContains<TypeList, typename Decay<T>::ValueT>::Value, Dummy>::ValueT* = 0)
		{
			base::Destruct();
			AssignVal(std::forward<T>(val));
		}

		variant& operator = (const variant& other)
		{
			assign(other);
			return *this;
		}

		variant& operator = (variant&& other)
		{
			assign(std::move(other));
			return *this;
		}

		template < typename T >
		typename EnableIf<TypeListContains<TypeList, typename Decay<T>::ValueT>::Value, variant&>::ValueT operator = (T&& val)
		{
			assign(std::forward<T>(val));
			return *this;
		}

		bool empty() const { return base::which() == IndexOfTypeListItem<TypeList, EmptyType>::Value; }

		bool operator == (const variant& other) const
		{
			typedef typename base::template ComparerVisitor<MyType, std::equal_to> VisitorType;
			if (this->which() != other.which())
				return false;
			return this->ApplyVisitor(VisitorType(other));
		}

		STINGRAYKIT_GENERATE_EQUALITY_OPERATORS_FROM_EQUAL(variant);

		bool operator < (const variant& other) const
		{
			typedef typename base::template ComparerVisitor<MyType, std::less> VisitorType;
			if (this->which() != other.which())
				return this->which() < other.which();
			return this->ApplyVisitor(VisitorType(other));
		}

		STINGRAYKIT_GENERATE_RELATIONAL_OPERATORS_FROM_LESS(variant);

		template < typename T, typename... Us >
		void emplace(Us&&... args)
		{
			base::Destruct();
			try
			{
				this->_storage.template Ctor<T>(std::forward<Us>(args)...);
				this->_type = IndexOfTypeListItem<TypeList, T>::Value;
			}
			catch (const std::exception& ex)
			{ AssignDefault(); throw; }
		}

	private:
		template < typename T >
		void AssignVal(T&& val, typename EnableIf<TypeListContains<TypeList, typename Decay<T>::ValueT>::Value, Dummy>::ValueT* = 0)
		{
			try
			{
				this->_storage.template Ctor<typename Decay<T>::ValueT>(std::forward<T>(val));
				this->_type = IndexOfTypeListItem<TypeList, typename Decay<T>::ValueT>::Value;
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
			{ }

			template < typename T >
			void operator () (const T& t) const { _target.AssignVal(t); }
		};

		void Assign(const MyType& other)
		{ other.ApplyVisitor(CopyCtorVisitor(*this)); }

		struct MoveCtorVisitor : static_visitor<>
		{
		private:
			MyType& _target;

		public:
			MoveCtorVisitor(MyType& t) : _target(t)
			{ }

			template < typename T >
			void operator () (T& t) const
			{
				static_assert(!IsConst<T>::Value, "Move won't work");
				_target.AssignVal(std::move(t));
			}
		};

		void Assign(MyType&& other)
		{ other.ApplyVisitor(MoveCtorVisitor(*this)); }
	};


	template < typename T, typename TypeList >
	T* variant_get(variant<TypeList>* v)
	{ return v->template get_ptr<T>(); }

	template < typename T, typename TypeList >
	const T* variant_get(const variant<TypeList>* v)
	{ return v->template get_ptr<T>(); }

	template < typename T, typename TypeList >
	T& variant_get(variant<TypeList>& v)
	{ return v.template get<T>(); }

	template < typename T, typename TypeList >
	const T& variant_get(const variant<TypeList>& v)
	{ return v.template get<const T>(); }

	template < typename Visitor, typename TypeList >
	typename Visitor::RetType apply_visitor(Visitor& visitor, variant<TypeList>& v)
	{ return v.ApplyVisitor(visitor); }

	template < typename Visitor, typename TypeList >
	typename Visitor::RetType apply_visitor(const Visitor& visitor, variant<TypeList>& v)
	{ return v.ApplyVisitor(visitor); }

	template < typename Visitor, typename TypeList >
	typename Visitor::RetType apply_visitor(Visitor& visitor, const variant<TypeList>& v)
	{ return v.ApplyVisitor(visitor); }

	template < typename Visitor, typename TypeList >
	typename Visitor::RetType apply_visitor(const Visitor& visitor, const variant<TypeList>& v)
	{ return v.ApplyVisitor(visitor); }


	template < typename Visitor >
	class VisitorApplier : public function_info<typename Visitor::RetType, UnspecifiedParamTypes>
	{
	private:
		Visitor		_visitor;

	public:
		explicit VisitorApplier(const Visitor& visitor) : _visitor(visitor) { }

		template < typename Variant >
		typename Visitor::RetType operator () (const Variant& v) const { return apply_visitor(_visitor, v); }
	};


	template < typename Visitor >
	VisitorApplier<Visitor> make_visitor_applier(const Visitor& visitor)
	{ return VisitorApplier<Visitor>(visitor); }

	/** @} */

}

#endif
