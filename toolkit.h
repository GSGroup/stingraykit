#ifndef STINGRAY_TOOLKIT_TOOLKIT_H
#define STINGRAY_TOOLKIT_TOOLKIT_H


#include <stdexcept>

#include <stingray/toolkit/MetaProgramming.h>
#include <stingray/toolkit/Types.h>
#include <stingray/toolkit/NestedTypeCheck.h>
#include <stingray/toolkit/EnumToString.h>

namespace stingray
{

	struct IFactoryObject
	{
		virtual ~IFactoryObject() { }

		virtual std::string GetClassName() const = 0;
	};

}

#if (__GNUC__ >= 3 || defined(__clang__)) && !defined(PRODUCTION_BUILD)
#	define TOOLKIT_FUNCTION __PRETTY_FUNCTION__
#else
#	define TOOLKIT_FUNCTION __func__
#endif


#ifdef __GNUC__
#	define STINGRAY_LIKELY(x)	__builtin_expect((x), 1)
#	define STINGRAY_UNLIKELY(x)	__builtin_expect((x), 0)
#else
#	define STINGRAY_LIKELY(x)	(x)
#	define STINGRAY_UNLIKELY(x)	(x)
#endif


namespace stingray
{
	struct ToolkitWhere
	{
		const char*	_file;
		size_t		_line;
		const char*	_functionName;

		ToolkitWhere(const char* file, int line, const char* functionName) : _file(file), _line(line), _functionName(functionName)
		{}

		size_t GetLine() const				{ return _line; }
		const char* GetFilename() const		{ return _file; }
		const char* GetFunctionName() const	{ return _functionName; }

		std::string ToString() const;
	};
}

// God damn C++!!!11 =(
#define TOOLKIT_WHERE (::stingray::ToolkitWhere(__FILE__, __LINE__, TOOLKIT_FUNCTION))

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

#define TOOLKIT_SIMPLE_ENUM_TO_STRING_BEGIN(EnumType) \
	std::string ToString(EnumType value) { \
		switch (value) { \
		default: return std::string("Unknown enum value " + stingray::ToString(static_cast<int>(value)) + " of " #EnumType);

#define TOOLKIT_SIMPLE_ENUM_VALUE(enumValue) case enumValue: return std::string(#enumValue)

#define TOOLKIT_SIMPLE_ENUM_TO_STRING_END() }}

#define TOOLKIT_ENUM_VALUES(...) \
	private: \
		inline static void InitEnumToStringMap(::stingray::Detail::EnumToStringMapBase& map) \
		{ \
			stingray::Detail::EnumValueHolder __VA_ARGS__; \
			stingray::Detail::EnumValueHolder values[] = { __VA_ARGS__ }; \
			map.DoInit(values, values + sizeof(values) / sizeof(values[0]), #__VA_ARGS__); \
		} \
	public: \
		enum Enum { __VA_ARGS__ }

#define TOOLKIT_DECLARE_ENUM_CLASS(ClassName) \
		friend class stingray::Detail::EnumToStringMapInstance<ClassName>; \
	public: \
		std::string ToString() const					{ return stingray::Detail::EnumToStringMap<ClassName>::EnumToString(_enumVal); } \
		static Enum FromString(const std::string& str)	{ return stingray::Detail::EnumToStringMap<ClassName>::EnumFromString(str); } \
		typedef stingray::Detail::EnumIterator<ClassName> const_iterator; \
		static const_iterator begin()					{ return stingray::Detail::EnumIteratorCreator<ClassName>::begin(); } \
		static const_iterator end()						{ return stingray::Detail::EnumIteratorCreator<ClassName>::end(); } \
		ClassName() { const std::vector<int>& v = stingray::Detail::EnumToStringMap<ClassName>::GetEnumValues(); _enumVal = v.empty() ? (Enum)0 : (Enum)v.front(); } \
		ClassName(Enum enumVal) : _enumVal(enumVal) { } \
		operator Enum () const { return _enumVal; } \
		Enum val() const { return _enumVal; } \
		template<typename T> inline bool operator<(T other) const { \
			stingray::CompileTimeAssert<stingray::SameType<ClassName, T>::Value> ERROR_invalid_enum_used; \
			return _enumVal < other._enumVal; \
		} \
		template<typename T> inline bool operator==(T other) const { \
			stingray::CompileTimeAssert<stingray::SameType<ClassName, T>::Value> ERROR_invalid_enum_used; \
			return _enumVal == other._enumVal; \
		} \
		inline bool operator==(Enum value) const { return _enumVal == value; } \
		template<typename T> inline bool operator!=(T other) const { \
			return !(*this == other); \
		} \
	private: \
		Enum _enumVal

#define TOOLKIT_DECLARE_ENUM_CLASS_BIT_OPERATORS(ClassName_) \
		inline ClassName_& operator |= (ClassName_& l, ClassName_::Enum r) \
		{ return l = ClassName_((ClassName_::Enum)((int)l | (int)r)); } \
		inline ClassName_& operator &= (ClassName_& l, ClassName_::Enum r) \
		{ return l = ClassName_((ClassName_::Enum)((int)l & (int)r)); } \
		inline ClassName_::Enum operator | (ClassName_::Enum l, ClassName_::Enum r) \
		{ ClassName_ result(l); return result |= r; } \
		inline ClassName_::Enum operator & (ClassName_::Enum l, ClassName_::Enum r) \
		{ ClassName_ result(l); return result &= r; }

#define TOOLKIT_GENERATE_COMPARISON_OPERATORS_FROM_LESS(ClassName) \
		inline bool operator > (const ClassName& other) const \
		{ return other < (*this); } \
		inline bool operator <= (const ClassName& other) const \
		{ return !(other < (*this)); } \
		inline bool operator >= (const ClassName& other) const \
		{ return !((*this) < other); } \
		inline bool operator != (const ClassName& other) const \
		{ return (other < (*this)) || ((*this) < other); } \
		inline bool operator == (const ClassName& other) const \
		{ return !(other != (*this)); }

#define TOOLKIT_GENERATE_EQUALITY_OPERATORS_FROM_EQUAL(ClassName) \
		inline bool operator != (const ClassName& other) const \
		{ return !(*this == other); }

#define TOOLKIT_GENERATE_RELATIONAL_OPERATORS_FROM_LESS(ClassName) \
		inline bool operator > (const ClassName& other) const \
		{ return other < *this; } \
		inline bool operator <= (const ClassName& other) const \
		{ return !(other < *this); } \
		inline bool operator >= (const ClassName& other) const \
		{ return !(*this < other); } \

#define TOOLKIT_GENERATE_FREE_COMPARISON_OPERATORS_FOR_TEMPLATE_CLASS(TemplateArgs, ClassName) \
		TemplateArgs inline bool operator <  (const ClassName& lhs, const ClassName& rhs) { return lhs <  rhs; } \
		TemplateArgs inline bool operator >  (const ClassName& lhs, const ClassName& rhs) { return lhs >  rhs; } \
		TemplateArgs inline bool operator <= (const ClassName& lhs, const ClassName& rhs) { return lhs <= rhs; } \
		TemplateArgs inline bool operator >= (const ClassName& lhs, const ClassName& rhs) { return lhs >= rhs; } \
		TemplateArgs inline bool operator != (const ClassName& lhs, const ClassName& rhs) { return lhs != rhs; } \
		TemplateArgs inline bool operator == (const ClassName& lhs, const ClassName& rhs) { return lhs == rhs; }

#define TOOLKIT_TRANSPARENT_DECORATOR_CTORS(DecoratorType_, BaseType_) \
		DecoratorType_() { } \
		template < typename T1 > DecoratorType_(const T1& p1) : BaseType_(p1) { }\
		template < typename T1, typename T2 > DecoratorType_(const T1& p1, const T2& p2) : BaseType_(p1, p2) { } \
		template < typename T1, typename T2, typename T3 > DecoratorType_(const T1& p1, const T2& p2, const T3& p3) : BaseType_(p1, p2, p3) { }


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

	struct EmptyType
	{
		bool operator < (const EmptyType &) const { return false; }
		TOOLKIT_GENERATE_COMPARISON_OPERATORS_FROM_LESS(EmptyType);
	};

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

	template<typename T> T implicit_cast(T param) { return param; }


	template < typename SrcType >
	struct InstanceOfTester
	{
		template < typename DestType >
		static inline bool Test(const SrcType& obj)
		{ return (dynamic_cast<const DestType*>(&obj) != 0); }
	};

	template < typename T >
	struct InstanceOfTester<T*>
	{
		template < typename DestType >
		static inline bool Test(const T* ptr)
		{ return (dynamic_cast<const DestType*>(ptr) != 0); }
	};

	template < typename DestType, typename SrcType >
	bool inline InstanceOf(const SrcType& obj)
	{ return InstanceOfTester<SrcType>::template Test<DestType>(obj); }

	template < typename T >
	class shared_ptr;

	template < typename DestType >
	struct InstanceOfPredicate
	{
		typedef bool RetType;

		template < typename Something > // There is also InstanceOf for shared_ptrs somewhere in shared_ptr.h
		inline bool operator () (const Something& obj) const
		{
			CompileTimeAssert<!Is1ParamTemplate<shared_ptr, DestType>::Value> ERROR_this_will_actually_test_your_instance_for_shared_ptr_This_is_probably_not_what_you_wanted;
			return InstanceOf<DestType>(obj);
		}
	};

	template < size_t N >
	size_t StrLen(const char (&)[N])
	{ return N - 1; }

	template < typename T, size_t Size >
	inline size_t ArraySize(const T (&) [Size]) { return Size; }


	template<typename T, size_t N>
	T* begin(T(&arr)[N])
	{ return &arr[0]; }


	template<typename T, size_t N>
	T* end(T(&arr)[N])
	{ return &arr[N]; }


	template<typename InputIterator, typename OutputIterator, typename Pred>
	OutputIterator copy_if(InputIterator first, InputIterator last, OutputIterator result, Pred pred)
	{
		for (; first != last; ++first)
			if (pred(*first))
				*result++ = *first;
		return result;
	}


	template<class InputIterator, class UnaryPredicate>
	bool any_of (InputIterator first, InputIterator last, UnaryPredicate pred)
	{
		while (first != last)
		{
			if (pred(*first))
				return true;
			++first;
		}
		return false;
	}


	struct CollectionOp
	{
		TOOLKIT_ENUM_VALUES
		(
			Added,
			Removed,
			Updated
		);
		TOOLKIT_DECLARE_ENUM_CLASS(CollectionOp);
	};

	struct DebuggingHelper
	{
		static void BreakpointHere();
#if defined(__GNUC__) || defined(__clang__)
		__attribute__((noreturn))
#endif
		static void TerminateWithMessage(const std::string& str) throw();
	};


	std::string Demangle(const std::string& s);

}


#endif
