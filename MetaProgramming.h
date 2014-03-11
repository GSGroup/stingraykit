#ifndef STINGRAY_TOOLKIT_METAPROGRAMMING_H
#define STINGRAY_TOOLKIT_METAPROGRAMMING_H


#include <stingray/toolkit/Macro.h>


namespace stingray
{


	class NullType;

	template < bool Expression > class CompileTimeAssert;
	template < > class CompileTimeAssert<true> { };


	struct YesType { char dummy; };
	struct NoType { YesType dummy[2]; };

	namespace
	{ CompileTimeAssert< sizeof(YesType) != sizeof(NoType) >	ERROR__yes_and_no_types_do_not_work; }


	template < bool Cond_, class T > struct EnableIf;
	template < class T> struct EnableIf<true, T> { typedef T	ValueT; };

	template < typename > struct ToVoid { typedef void ValueT; };

	template < int N >
	struct IntToType
	{ static const int Value = N; };


	template < template <typename> class Predicate >
	struct Not
	{
		template < typename T >
		struct ValueT { static const bool Value = !Predicate<T>::Value; };
	};


	template < typename T, typename U > struct SameType { static const bool Value = false; };
	template < typename T > struct SameType<T, T> { static const bool Value = true; };

	template < template <typename> class Template, typename U >
	struct Is1ParamTemplate { static const bool Value = false; };
	template < template <typename> class Template, typename T >
	struct Is1ParamTemplate<Template, Template<T> > { static const bool Value = true; };

	template < template <typename, typename> class Template, typename U >
	struct Is2ParamTemplate { static const bool Value = false; };
	template < template <typename, typename> class Template, typename T, typename U >
	struct Is2ParamTemplate<Template, Template<T, U> > { static const bool Value = true; };

	template < typename T, typename U>
	struct CanCast
	{
		static YesType Test(bool);
		static NoType Test(...);

		static const bool Value = sizeof(Test(*((const T*)0))) == sizeof(YesType);
	};

	template < typename T >
	struct IsReference { static const bool Value = false; };

	template < typename T >
	struct IsReference<T&> { static const bool Value = true; };

	template < typename T >
	struct IsConstReference { static const bool Value = false; };

	template < typename T >
	struct IsConstReference<const T&> { static const bool Value = true; };

	template < typename T >
	struct IsNonConstReference { static const bool Value = IsReference<T>::Value && !IsConstReference<T>::Value; };

	template < typename T >
	struct IsConst { static const bool Value = false; };

	template < typename T >
	struct IsConst<const T> { static const bool Value = true; };

	template < typename T >
	struct IsPointer { static const bool Value = false; };

	template < typename T >
	struct IsPointer<T*> { static const bool Value = true; };

	template < typename T >
	struct IsArray { static const bool Value = false; };

	template < typename T, unsigned N >
	struct IsArray<T[N]> { static const bool Value = true; };

	template < typename T, typename MemberPointerDetector = void >
	struct IsClass
	{ static const bool Value = false; };

	template < typename T >
	struct IsClass<T, typename ToVoid<int T::*>::ValueT >
	{ static const bool Value = true; };

	template < bool Expression, typename IfType, typename ElseType >
	struct If;

	template < typename IfType, typename ElseType >
	struct If<true, IfType, ElseType> { typedef IfType ValueT; };

	template < typename IfType, typename ElseType >
	struct If<false, IfType, ElseType> { typedef ElseType ValueT; };


	template < typename T >
	struct TypeWrapper
	{ typedef T ValueT; };


	template < typename Derived, typename Base>
	class Inherits
	{
	private:
		static YesType TestInheritance(const Base*);
		static NoType TestInheritance(...);

	public:
		static const bool Value = ( sizeof(TestInheritance((const Derived*)0)) == sizeof(YesType) );
	};


	template < typename T >
	struct GetConstReferenceType
	{ typedef const T&	ValueT; };

	template < typename T >
	struct GetConstReferenceType<T&>
	{ typedef const T&	ValueT; };

	template < typename T >
	struct GetConstReferenceType<const T&>
	{ typedef const T&	ValueT; };

	template < typename T >
	struct GetConstPointerType
	{ typedef const T*	ValueT; };

	template < typename T >
	struct GetPointedType;

	template < typename T >
	struct GetPointedType<T *>
	{ typedef T ValueT; };

	template < typename T >
	struct Dereference { typedef T	ValueT; };

	template < typename T >
	struct Dereference<T&> { typedef T	ValueT; };

	template < typename T >
	struct Depointer { typedef T	ValueT; };

	template < typename T >
	struct Depointer<T*> { typedef T	ValueT; };

	template<typename T>
	struct Deconst { typedef T ValueT; };

	template<typename T>
	struct Deconst<const T> { typedef T ValueT; };


	template<typename T_, T_ Value_>
	struct integral_constant
	{
		typedef T_ ValueT;
		static const ValueT Value = Value_;
	};

	template<typename Val1, typename Val2>
	struct integral_constant_less
	{ static const bool Value = Val1::Value < Val2::Value; };


	template<typename ResultT, ResultT Base, unsigned int Exponent>
	struct CompileTimeExponent { static const ResultT Value = Base * CompileTimeExponent<ResultT, Base, Exponent - 1>::Value; };

	template<typename ResultT, ResultT Base>
	struct CompileTimeExponent<ResultT, Base, 0> { static const ResultT Value = 1; };



#define TY typename
	template < unsigned Count, template <int> class FunctorClass, int Start = 0 >
	struct For
	{
#define DETAIL_TOOLKIT_DECLARE_FOR_DO(N_, UserArg_) \
		TOOLKIT_INSERT_IF(N_, template <) TOOLKIT_REPEAT(N_, TOOLKIT_TEMPLATE_PARAM_DECL, ~) TOOLKIT_INSERT_IF(N_, >) \
		static void Do(TOOLKIT_REPEAT(N_, TOOLKIT_FUNCTION_PARAM_DECL, ~)) \
		{ \
			FunctorClass<Start>::Call(TOOLKIT_REPEAT(N_, TOOLKIT_FUNCTION_PARAM_USAGE, ~)); \
			For<Count - 1, FunctorClass, Start + 1>:: TOOLKIT_INSERT_IF(N_, template) Do TOOLKIT_INSERT_IF(N_, <) TOOLKIT_REPEAT(N_, TOOLKIT_TEMPLATE_PARAM_USAGE, ~) TOOLKIT_INSERT_IF(N_, >) (TOOLKIT_REPEAT(N_, TOOLKIT_FUNCTION_PARAM_USAGE, ~)); \
		}

		TOOLKIT_REPEAT_NESTING_2(10, DETAIL_TOOLKIT_DECLARE_FOR_DO, ~)

#undef DETAIL_TOOLKIT_DECLARE_FOR_DO
	};

	template < template <int> class FunctorClass, int Start >
	struct For<0, FunctorClass, Start>
	{
#define DETAIL_TOOLKIT_DECLARE_FOR_DO(N_, UserArg_) \
		TOOLKIT_INSERT_IF(N_, template <) TOOLKIT_REPEAT(N_, TOOLKIT_TEMPLATE_PARAM_DECL, ~) TOOLKIT_INSERT_IF(N_, >) \
		static void Do(TOOLKIT_REPEAT(N_, TOOLKIT_FUNCTION_PARAM_DECL, ~)) \
		{ }

		TOOLKIT_REPEAT_NESTING_2(10, DETAIL_TOOLKIT_DECLARE_FOR_DO, ~)

#undef DETAIL_TOOLKIT_DECLARE_FOR_DO
	};

	template < unsigned Count, template <int> class FunctorClass, int Start = 0 >
	struct ForIf
	{
		static bool Do()
		{
			bool success = FunctorClass<Start>::Call();
			return success && ForIf<Count - 1, FunctorClass, Start + 1>::Do();
		}

#define DETAIL_TOOLKIT_DECLARE_FOR_IF_DO(TypesDecl_, TypesUsage_, ParamsDecl_, ParamsUsage_) \
		template < TypesDecl_ > \
		static bool Do(ParamsDecl_) \
		{ \
			bool success = FunctorClass<Start>::Call(ParamsUsage_); \
			return success && ForIf<Count - 1, FunctorClass, Start + 1>::template Do<TypesUsage_>(ParamsUsage_); \
		}

		DETAIL_TOOLKIT_DECLARE_FOR_IF_DO(MK_PARAM(TY T1), MK_PARAM(T1), MK_PARAM(const T1& p1), MK_PARAM(p1));
		DETAIL_TOOLKIT_DECLARE_FOR_IF_DO(MK_PARAM(TY T1, TY T2), MK_PARAM(T1, T2), MK_PARAM(const T1& p1, const T2& p2), MK_PARAM(p1, p2));
		DETAIL_TOOLKIT_DECLARE_FOR_IF_DO(MK_PARAM(TY T1, TY T2, TY T3), MK_PARAM(T1, T2, T3), MK_PARAM(const T1& p1, const T2& p2, const T3& p3), MK_PARAM(p1, p2, p3));

#undef DETAIL_TOOLKIT_DECLARE_FOR_IF_DO
	};

	template < template <int> class FunctorClass, int Start >
	struct ForIf<0, FunctorClass, Start>
	{
		static bool Do() { return true; }

		template <TY T1> static bool Do(const T1&) { return true; }
		template <TY T1, TY T2> static bool Do(const T1&, const T2&) { return true; }
		template <TY T1, TY T2, TY T3> static bool Do(const T1&, const T2&, const T3&) { return true; }
	};


#undef TY


}


#endif
