#ifndef __GS_DVRLIB_TOOLKIT_NESTEDTYPECHECK_H__
#define __GS_DVRLIB_TOOLKIT_NESTEDTYPECHECK_H__


#include <stingray/toolkit/TypeList.h>

namespace stingray
{

	namespace Detail
	{
		struct DoesNotHaveAnyNestedTypes
		{ static const bool Value = false; };
	}

#define TOOLKIT_DECLARE_NESTED_TYPE_CHECK(NestedType_) \
	template < typename T > \
	struct HasNestedType_##NestedType_ \
	{ \
		template < typename Type > \
		class Impl \
		{ \
			struct BaseMixin { struct NestedType_{}; }; \
			struct Base : public Type, public BaseMixin { Base(); }; \
			\
			template <typename V, V t>    class Helper{}; \
			\
			template <typename U> static NoType deduce(U*, typename U::NestedType_* = 0); \
			static YesType deduce(...); \
			\
		public: \
			static const bool Value = (sizeof(YesType) == sizeof(deduce((Base*)(0)))); \
		};\
		static const bool Value = \
			If	< \
					IsClass<T>::Value, \
					Impl<T>, \
					stingray::Detail::DoesNotHaveAnyNestedTypes \
				>::ValueT::Value; \
	}

#define TOOLKIT_DECLARE_METHOD_CHECK(Method_) \
	template < typename T > \
	struct HasMethod_##Method_ \
	{ \
		template <typename Type> \
		class Impl \
		{ \
			struct BaseMixin { void Method_(){} }; \
			struct Base : public Type, public BaseMixin { Base(); }; \
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
