#ifndef STINGRAYKIT_TOOLKIT_H
#define STINGRAYKIT_TOOLKIT_H

// Copyright (c) 2011 - 2024, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/MetaProgramming.h>
#include <stingraykit/operators.h>

#include <string>

#if (defined(__GNUC__) || defined(__clang__)) && !defined(PRODUCTION_BUILD)
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
		{ }

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

namespace stingray
{

	template < typename T > const T* to_pointer(const T& val) { return &val; }
	template < typename T > T* to_pointer(T& val) { return &val; }
	template < typename T > T* to_pointer(T* ptr) { return ptr; }

	struct EmptyType
	{
		bool operator < (const EmptyType&) const { return false; }
		STINGRAYKIT_GENERATE_COMPARISON_OPERATORS_FROM_LESS(EmptyType);
	};


	template < typename T >
	struct StaticCaster
	{
		using RetType = T;

		template < typename U >
		T operator () (U param) const
		{ return static_cast<T>(param); }
	};


	template < typename T >
	struct ImplicitCaster
	{
		using RetType = T;

		T operator () (T param) const
		{ return param; }
	};


	template < typename T > T implicit_cast(T param) { return param; }


	template < typename SrcType >
	struct InstanceOfTester
	{
		template < typename DestType >
		static bool Test(const SrcType& obj)
		{ return (dynamic_cast<const DestType*>(&obj) != 0); }
	};


	template < typename T >
	struct InstanceOfTester<T*>
	{
		template < typename DestType >
		static bool Test(const T* ptr)
		{ return (dynamic_cast<const DestType*>(ptr) != 0); }
	};


	template < typename DestType, typename SrcType >
	bool InstanceOf(const SrcType& obj)
	{ return InstanceOfTester<SrcType>::template Test<DestType>(obj); }


	template < typename T >
	class shared_ptr;


	template < typename DestType >
	struct InstanceOfPredicate
	{
		using RetType = bool;

		template < typename Something > // There is also InstanceOf for shared_ptrs somewhere in shared_ptr.h
		bool operator () (const Something& obj) const
		{
			static_assert(!Is1ParamTemplate<shared_ptr, DestType>::Value, "This will actually test your instance for shared_ptr. This is probably not what you wanted");
			return InstanceOf<DestType>(obj);
		}
	};


	template < size_t N >
	size_t StrLen(const char (&)[N])
	{ return N - 1; }


	template < typename T, size_t Size >
	size_t ArraySize(const T (&) [Size]) { return Size; }


	namespace Detail
	{
		void ArrayCheckRange(size_t index, size_t size);
	}

	template < typename T, size_t Size >
	T& ArrayGet(T (&array) [Size], size_t index)
	{
		Detail::ArrayCheckRange(index, Size);
		return array[index];
	}


	struct DebuggingHelper
	{
		static void BreakpointHere();
#if defined(__GNUC__) || defined(__clang__)
		__attribute__((noreturn))
#endif
		static void TerminateWithMessage(ToolkitWhere where, const std::string& message) noexcept;
	};


	std::string Demangle(const std::string& s);


	template < typename T >
	struct IsNullable : public FalseType { };

	template < typename T >
	struct IsNullable<T*> : public TrueType { };

}


#endif
