#ifndef __GS_DVRLIB_TOOLKIT_TOOLKIT_H__
#define __GS_DVRLIB_TOOLKIT_TOOLKIT_H__


#include <stdexcept>

#include <stingray/toolkit/MetaProgramming.h>
#include <stingray/toolkit/Types.h>
#include <stingray/toolkit/NestedTypeCheck.h>
#include <stingray/toolkit/EnumToString.h>
#include <stingray/toolkit/Dummy.h>

namespace stingray
{
	struct IFactoryObjectCreator;
	struct IFactoryObject
	{
		virtual const IFactoryObjectCreator& GetFactoryObjectCreator() const = 0;
		virtual ~IFactoryObject() {}
	};
}

#if (__GNUC__ >= 3) && !defined(PRODUCTION_BUILD)
#	define TOOLKIT_FUNCTION __PRETTY_FUNCTION__
#else
#	define TOOLKIT_FUNCTION __func__
#endif

// God damn C++!!!11 =(
#define TOOLKIT_WHERE ((std::string("function '") + TOOLKIT_FUNCTION + "' in file '" __FILE__ "' at line " TO_STRING(__LINE__)))

#ifdef DEBUG
#	define TOOLKIT_DEBUG_ONLY(...) __VA_ARGS__
#else
#	define TOOLKIT_DEBUG_ONLY(...)
#endif


#define TOOLKIT_NONCOPYABLE(ClassName) \
	private: \
		ClassName(const ClassName&); \
		ClassName& operator= (const ClassName&)

#define TOOLKIT_NONASSIGNABLE(ClassName) \
	private: \
		ClassName& operator= (const ClassName&)

#define TOOLKIT_ENUM_VALUES(...) \
	public:\
		enum Enum { __VA_ARGS__ }; \
	private: \
		template < typename MyType > \
		FORCE_INLINE static void InitEnumToStringMap(const MyType* dummy) \
		{ \
			if (!stingray::Detail::GetEnumToStringMap(dummy).Initialized()) \
			{ \
				stingray::Detail::EnumValueHolder __VA_ARGS__; \
				stingray::Detail::EnumValueHolder values[] = { __VA_ARGS__ }; \
				stingray::Detail::GetEnumToStringMap(dummy).Init(values, values + sizeof(values) / sizeof(values[0]), #__VA_ARGS__); \
			} \
		} \
	public: \
		std::string ToString() const { InitEnumToStringMap(this); return stingray::Detail::GetEnumToStringMap(this).EnumToString(_enumVal); } \
		static Enum FromString(const std::string& str) { InitEnumToStringMap(GetMyTypeDummyPtr()); return stingray::Detail::GetEnumToStringMap(GetMyTypeDummyPtr()).EnumFromString(str); }

#define TOOLKIT_DECLARE_ENUM_CLASS(ClassName) \
		friend class stingray::Detail::EnumToStringMap<ClassName>; \
	public: \
		typedef stingray::Detail::EnumIterator<ClassName> const_iterator; \
		static const_iterator begin()	{ InitEnumToStringMap(GetMyTypeDummyPtr()); return stingray::Detail::EnumIteratorCreator<ClassName>::begin(); } \
		static const_iterator end()		{ InitEnumToStringMap(GetMyTypeDummyPtr()); return stingray::Detail::EnumIteratorCreator<ClassName>::end(); } \
		ClassName() { InitEnumToStringMap(this); const std::vector<Enum>& v = stingray::Detail::GetEnumToStringMap(this).GetEnumValues(); _enumVal = v.empty() ? (Enum)0 : v.front(); } \
		ClassName(Enum enumVal) : _enumVal(enumVal) { } \
		operator Enum () const { return _enumVal; } \
		ClassName::Enum val() const { return _enumVal; } \
	private: \
		FORCE_INLINE static const ClassName* GetMyTypeDummyPtr() { return NULL; } \
		ClassName::Enum _enumVal

#define TOOLKIT_DECLARE_ENUM_CLASS_BIT_OPERATORS(ClassName_) \
		inline ClassName_::Enum operator | (ClassName_::Enum l, ClassName_::Enum r) \
		{ return ClassName_(ClassName_::Enum(((int)l | (int)r))); } \
		inline ClassName_::Enum operator & (ClassName_::Enum l, ClassName_::Enum r) \
		{ return ClassName_(ClassName_::Enum(((int)l & (int)r))); }

#define TOOLKIT_GENERATE_COMPARISON_OPERATORS_FROM_LESS(ClassName) \
		FORCE_INLINE bool operator > (const ClassName& other) const \
		{ return other < (*this); } \
		FORCE_INLINE bool operator <= (const ClassName& other) const \
		{ return !(other < (*this)); } \
		FORCE_INLINE bool operator >= (const ClassName& other) const \
		{ return !((*this) < other); } \
		FORCE_INLINE bool operator != (const ClassName& other) const \
		{ return (other < (*this)) || ((*this) < other); } \
		FORCE_INLINE bool operator == (const ClassName& other) const \
		{ return !(other != (*this)); }

#define TOOLKIT_GENERATE_FREE_COMPARISON_OPERATORS_FOR_TEMPLATE_CLASS(TemplateArgs, ClassName) \
		TemplateArgs FORCE_INLINE bool operator <  (const ClassName& lhs, const ClassName& rhs) { return lhs <  rhs; } \
		TemplateArgs FORCE_INLINE bool operator >  (const ClassName& lhs, const ClassName& rhs) { return lhs >  rhs; } \
		TemplateArgs FORCE_INLINE bool operator <= (const ClassName& lhs, const ClassName& rhs) { return lhs <= rhs; } \
		TemplateArgs FORCE_INLINE bool operator >= (const ClassName& lhs, const ClassName& rhs) { return lhs >= rhs; } \
		TemplateArgs FORCE_INLINE bool operator != (const ClassName& lhs, const ClassName& rhs) { return lhs != rhs; } \
		TemplateArgs FORCE_INLINE bool operator == (const ClassName& lhs, const ClassName& rhs) { return lhs == rhs; }

#define TOOLKIT_TRANSPARENT_DECORATOR_CTORS(DecoratorType_, BaseType_) \
		DecoratorType_() { } \
		template < typename T1 > DecoratorType_(const T1& p1) : BaseType_(p1) { }\
		template < typename T1, typename T2 > DecoratorType_(const T1& p1, const T2& p2) : BaseType_(p1, p2) { } \
		template < typename T1, typename T2, typename T3 > DecoratorType_(const T1& p1, const T2& p2, const T3& p3) : BaseType_(p1, p2, p3) { }

#if defined __GNUC__ && !defined __GNUC_STDC_INLINE__ && !defined __GNUC_GNU_INLINE__
#	define __GNUC_GNU_INLINE__ 1
#endif

#if __GNUC_GNU_INLINE__ && !defined(FORCE_INLINE_DISABLED)
#	define FORCE_INLINE __attribute__((always_inline))
#else
#	define FORCE_INLINE inline
#endif

namespace stingray
{

	template < typename >
	struct ToPointerType;

	template < typename T >
	struct ToPointerType<T&>
	{ typedef T* ValueT; };

	template < typename T >
	T* to_pointer(T& val)
	{ return &val; }


	TOOLKIT_DECLARE_NESTED_TYPE_CHECK(Enum);

	template < typename T >
	struct IsEnumClass { static const bool Value = HasNestedType_Enum<T>::Value; };


	struct NullPtrType
	{
		template < typename T >
		operator T* () const
		{ return 0; }
	};

	extern NullPtrType null;

	/*! \cond GS_INTERNAL */

	template < typename >
	class shared_ptr;

	template < typename SrcType >
	struct InstanceOfTester
	{
		template < typename DestType >
		static FORCE_INLINE bool Test(const SrcType& obj)
		{ return (dynamic_cast<const DestType*>(&obj) != 0); }
	};

	template < typename T >
	struct InstanceOfTester<T*>
	{
		template < typename DestType >
		static FORCE_INLINE bool Test(const T* ptr)
		{ return (dynamic_cast<const DestType*>(ptr) != 0); }
	};

	template < typename DestType, typename SrcType >
	bool FORCE_INLINE InstanceOf(const SrcType& obj)
	{
		CompileTimeAssert<!Is1ParamTemplate<shared_ptr, DestType>::Value>		ERROR__dest_type_must_not_be_a_shared_ptr;
		(void)ERROR__dest_type_must_not_be_a_shared_ptr;
		return InstanceOfTester<SrcType>::template Test<DestType>(obj);
	}


	template < typename DestType >
	struct InstanceOfPredicate
	{
		template < typename Something > // There is also InstanceOf for shared_ptrs somewhere in shared_ptr.h
		FORCE_INLINE bool operator () (const Something& obj) const
		{ return InstanceOf<DestType>(obj); }
	};

	template<typename ArrayType>
	FORCE_INLINE size_t ArraySize(const ArrayType& src) {
		return sizeof(src) / sizeof(src[0]);
	}

	template<typename InputIterator, typename OutputIterator, typename Pred>
	OutputIterator copy_if(InputIterator first, InputIterator last, OutputIterator result, Pred pred)
	{
		while (first != last)
			if (pred(*first))
				*result++ = *first++;
		return result;
	}

	/*! \endcond */

	struct CollectionOp
	{
		TOOLKIT_ENUM_VALUES
		(
			ItemAdded,
			ItemRemoved
		);
		TOOLKIT_DECLARE_ENUM_CLASS(CollectionOp);
	};

	struct DebuggingHelper
	{
		static void BreakpointHere();
	};


}


#endif
