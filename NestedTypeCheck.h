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
	namespace Detail \
	{ \
		template < typename Type > \
		class HasNestedType_##NestedType_##_Impl \
		{ \
			struct BaseMixin { struct NestedType_{}; }; \
			struct Base : public Type, public BaseMixin { Base(); }; \
			\
			template <typename T, T t>    class Helper{}; \
			\
			template <typename U> static NoType deduce(U*, typename U::NestedType_* = 0); \
			static YesType deduce(...); \
			\
		public: \
			static const bool Value = (sizeof(YesType) == sizeof(deduce((Base*)(0)))); \
		};\
	} \
	template < typename T > \
	struct HasNestedType_##NestedType_ \
	{ \
		static const bool Value = \
			If	< \
					IsClass<T>::Value, \
					Detail::HasNestedType_##NestedType_##_Impl<T>, \
					stingray::Detail::DoesNotHaveAnyNestedTypes \
				>::ValueT::Value; \
	}

#define TOOLKIT_DECLARE_METHOD_CHECK(Method_) \
	namespace Detail \
	{ \
		template <typename Type> \
		class HasMethod_##Method_##_Impl \
		{ \
			struct BaseMixin { void Method_(){} }; \
			struct Base : public Type, public BaseMixin { Base(); }; \
			\
			template <typename T, T t>    class Helper{}; \
			\
			template <typename U> static NoType deduce(U*, Helper<void (BaseMixin::*)(), &U::Method_>* = 0); \
			static YesType deduce(...); \
			\
		public: \
			static const bool Value = (sizeof(YesType) == sizeof(deduce((Base*)(0)))); \
		}; \
	} \
	template < typename T > \
	struct HasMethod_##Method_ \
	{ \
		static const bool Value = \
			If	< \
					IsClass<T>::Value, \
					Detail::HasMethod_##Method_##_Impl<T>, \
					stingray::Detail::DoesNotHaveAnyNestedTypes \
				>::ValueT::Value; \
	}

}


#endif
