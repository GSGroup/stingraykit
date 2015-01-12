#ifndef STINGRAY_TOOLKIT_NESTEDTYPECHECK_H
#define STINGRAY_TOOLKIT_NESTEDTYPECHECK_H


#include <stingray/toolkit/MetaProgramming.h>


namespace stingray
{

#define STINGRAYKIT_DECLARE_NESTED_TYPE_CHECK(NestedType_) \
	template < typename T > \
	class HasNestedType_##NestedType_ \
	{ \
		template < typename U > static YesType deduce(IntToType<sizeof(typename Dereference<typename U::NestedType_>::ValueT*)>*); \
		template < typename U > static NoType deduce(...); \
		\
	public: \
		static const bool Value = sizeof(deduce<T>(0)) == sizeof(YesType); \
	}

	namespace Detail
	{
		struct DoesNotHaveAnyNestedTypes
		{ static const bool Value = false; };
	}

#define STINGRAYKIT_DECLARE_METHOD_CHECK(Method_) \
	template < typename T > \
	struct HasMethod_##Method_ \
	{ \
		template <typename Type_> \
		class Impl \
		{ \
			struct BaseMixin { void Method_(){} }; \
			struct Base : public Type_, public BaseMixin { Base(); }; \
			\
			template <typename V, V t>    class Helper{}; \
			\
			template <typename U> static NoType deduce(U*, Helper<void (BaseMixin::*)(), &U::Method_>* = 0); \
			static YesType deduce(...); \
			\
		public: \
			static const bool Value = (sizeof(YesType) == sizeof(deduce((Base*)(0)))); \
		}; \
		static const bool Value = \
			If	< \
					IsClass<T>::Value, \
					Impl<T>, \
					stingray::Detail::DoesNotHaveAnyNestedTypes \
				>::ValueT::Value; \
	}

}


#endif
