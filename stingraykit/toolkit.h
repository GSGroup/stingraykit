#ifndef STINGRAYKIT_TOOLKIT_H
#define STINGRAYKIT_TOOLKIT_H

// Copyright (c) 2011 - 2017, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/EnumToString.h>
#include <stingraykit/MetaProgramming.h>
#include <stingraykit/NullPtrType.h>
#include <stingraykit/Types.h>
#include <stingraykit/core/NonCopyable.h>
#include <stingraykit/metaprogramming/NestedTypeCheck.h>

#include <stdexcept>

namespace stingray
{

	struct IFactoryObject
	{
		virtual ~IFactoryObject() { }

		virtual std::string GetClassName() const = 0;
	};

}

#if (__GNUC__ >= 3 || defined(__clang__)) && !defined(PRODUCTION_BUILD)
#	define STINGRAYKIT_FUNCTION __PRETTY_FUNCTION__
#else
#	define STINGRAYKIT_FUNCTION __func__
#endif


#ifdef __GNUC__
#	define STINGRAYKIT_LIKELY(x)	__builtin_expect((x), 1)
#	define STINGRAYKIT_UNLIKELY(x)	__builtin_expect((x), 0)
#else
#	define STINGRAYKIT_LIKELY(x)	(x)
#	define STINGRAYKIT_UNLIKELY(x)	(x)
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
#define STINGRAYKIT_WHERE (::stingray::ToolkitWhere(__FILE__, __LINE__, STINGRAYKIT_FUNCTION))

#ifdef DEBUG
#	define STINGRAYKIT_DEBUG_ONLY(...) __VA_ARGS__
#else
#	define STINGRAYKIT_DEBUG_ONLY(...)
#endif


#define STINGRAYKIT_SIMPLE_ENUM_TO_STRING_BEGIN(EnumType) \
	std::string ToString(EnumType value) { \
		switch (value) { \
		default: return std::string("Unknown enum value " + stingray::ToString(static_cast<int>(value)) + " of " #EnumType);

#define STINGRAYKIT_SIMPLE_ENUM_VALUE(enumValue) case enumValue: return std::string(#enumValue)

#define STINGRAYKIT_SIMPLE_ENUM_TO_STRING_END() }}

#define STINGRAYKIT_ENUM_VALUES(...) \
	private: \
		inline static void InitEnumToStringMap(::stingray::Detail::EnumToStringMapBase& map) \
		{ \
			stingray::Detail::EnumValueHolder __VA_ARGS__; \
			stingray::Detail::EnumValueHolder values[] = { __VA_ARGS__ }; \
			map.DoInit(values, values + sizeof(values) / sizeof(values[0]), #__VA_ARGS__); \
		} \
	public: \
		enum Enum { __VA_ARGS__ }

#define STINGRAYKIT_DECLARE_ENUM_CLASS(ClassName) \
		friend class stingray::Detail::EnumToStringMapInstance<ClassName>; \
	public: \
		std::string ToString() const					{ return stingray::Detail::EnumToStringMap<ClassName>::EnumToString(_enumVal); } \
		static Enum FromString(const std::string& str)	{ return stingray::Detail::EnumToStringMap<ClassName>::EnumFromString(str); } \
		typedef stingray::Detail::EnumIterator<ClassName> const_iterator; \
		static const_iterator begin()					{ return stingray::Detail::EnumIteratorCreator<ClassName>::begin(); } \
		static const_iterator end()						{ return stingray::Detail::EnumIteratorCreator<ClassName>::end(); } \
		ClassName() : _enumVal() { const std::vector<int>& v = stingray::Detail::EnumToStringMap<ClassName>::GetEnumValues(); if (!v.empty()) _enumVal = static_cast<Enum>(v.front()); } \
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

#define STINGRAYKIT_DECLARE_ENUM_CLASS_BIT_OPERATORS(ClassName_) \
		inline ClassName_& operator |= (ClassName_& l, ClassName_::Enum r) \
		{ return l = ClassName_(static_cast<ClassName_::Enum>(static_cast<int>(l) | static_cast<int>(r))); } \
		inline ClassName_& operator &= (ClassName_& l, ClassName_::Enum r) \
		{ return l = ClassName_(static_cast<ClassName_::Enum>(static_cast<int>(l) & static_cast<int>(r))); } \
		inline ClassName_::Enum operator | (ClassName_::Enum l, ClassName_::Enum r) \
		{ ClassName_ result(l); return result |= r; } \
		inline ClassName_::Enum operator & (ClassName_::Enum l, ClassName_::Enum r) \
		{ ClassName_ result(l); return result &= r; }

#define STINGRAYKIT_GENERATE_COMPARISON_OPERATORS_FROM_LESS(ClassName) \
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

#define STINGRAYKIT_GENERATE_COMPARISON_OPERATORS_FROM_COMPARE(ClassName) \
		inline bool operator <  (const ClassName& other) const { return Compare(other) <  0; } \
		inline bool operator >  (const ClassName& other) const { return Compare(other) >  0; } \
		inline bool operator <= (const ClassName& other) const { return Compare(other) <= 0; } \
		inline bool operator >= (const ClassName& other) const { return Compare(other) >= 0; } \
		inline bool operator == (const ClassName& other) const { return Compare(other) == 0; } \
		inline bool operator != (const ClassName& other) const { return Compare(other) != 0; }

#define STINGRAYKIT_GENERATE_EQUALITY_OPERATORS_FROM_EQUAL(ClassName) \
		inline bool operator != (const ClassName& other) const \
		{ return !(*this == other); }

#define STINGRAYKIT_GENERATE_RELATIONAL_OPERATORS_FROM_LESS(ClassName) \
		inline bool operator > (const ClassName& other) const \
		{ return other < *this; } \
		inline bool operator <= (const ClassName& other) const \
		{ return !(other < *this); } \
		inline bool operator >= (const ClassName& other) const \
		{ return !(*this < other); } \

#define STINGRAYKIT_GENERATE_FREE_COMPARISON_OPERATORS_FOR_TEMPLATE_CLASS(TemplateArgs, ClassName) \
		TemplateArgs inline bool operator <  (const ClassName& lhs, const ClassName& rhs) { return lhs <  rhs; } \
		TemplateArgs inline bool operator >  (const ClassName& lhs, const ClassName& rhs) { return lhs >  rhs; } \
		TemplateArgs inline bool operator <= (const ClassName& lhs, const ClassName& rhs) { return lhs <= rhs; } \
		TemplateArgs inline bool operator >= (const ClassName& lhs, const ClassName& rhs) { return lhs >= rhs; } \
		TemplateArgs inline bool operator != (const ClassName& lhs, const ClassName& rhs) { return lhs != rhs; } \
		TemplateArgs inline bool operator == (const ClassName& lhs, const ClassName& rhs) { return lhs == rhs; }

#define STINGRAYKIT_TRANSPARENT_DECORATOR_CTORS(DecoratorType_, BaseType_) \
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
	struct ToPointerType<T*>
	{ typedef T* ValueT; };

	template < typename T > T* to_pointer(T& val) { return &val; }
	template < typename T > T* to_pointer(T* ptr) { return ptr; }

	struct EmptyType
	{
		bool operator < (const EmptyType &) const { return false; }
		STINGRAYKIT_GENERATE_COMPARISON_OPERATORS_FROM_LESS(EmptyType);
	};

	STINGRAYKIT_DECLARE_NESTED_TYPE_CHECK(Enum);

	template < typename T >
	struct IsEnumClass { static const bool Value = HasNestedType_Enum<T>::Value; };


	template<typename T>
	struct ImplicitCaster
	{
		typedef T RetType;

		T operator () (T param) const
		{ return param; }
	};


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


	struct CollectionOp
	{
		STINGRAYKIT_ENUM_VALUES
		(
			Added,
			Removed,
			Updated
		);
		STINGRAYKIT_DECLARE_ENUM_CLASS(CollectionOp);
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
