#ifndef STINGRAYKIT_EXCEPTION_H
#define STINGRAYKIT_EXCEPTION_H

// Copyright (c) 2011 - 2019, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/diagnostics/Backtrace.h>
#include <stingraykit/string/ToStringForward.h>
#include <stingraykit/toolkit.h>

#include <stdexcept>
#include <typeinfo>

#define STINGRAYKIT_DECLARE_SIMPLE_EXCEPTION(ExceptionClass, Message) \
	struct ExceptionClass : public stingray::Exception \
	{ \
		ExceptionClass() : stingray::Exception(Message) { } \
		explicit ExceptionClass(const std::string& message) : stingray::Exception(message + ": " + Message) { } \
	}

namespace stingray
{

	class Exception : public std::runtime_error
	{
	public:
		explicit Exception(const std::string& message) : std::runtime_error(message) { }
	};

	STINGRAYKIT_DECLARE_SIMPLE_EXCEPTION(NotImplementedException, "The feature is not implemented!");
	STINGRAYKIT_DECLARE_SIMPLE_EXCEPTION(NotSupportedException, "The feature is not supported!");
	STINGRAYKIT_DECLARE_SIMPLE_EXCEPTION(DeviceBusyException, "Device is busy!");
	STINGRAYKIT_DECLARE_SIMPLE_EXCEPTION(ResourceUnavailableException, "Resource temporarily unavailable!");
	STINGRAYKIT_DECLARE_SIMPLE_EXCEPTION(BrokenPromise, "Promise destroyed before value is set!");
	STINGRAYKIT_DECLARE_SIMPLE_EXCEPTION(PromiseAlreadySatisfied, "Promise value have already been set!");
	STINGRAYKIT_DECLARE_SIMPLE_EXCEPTION(FutureAlreadyRetrieved, "Future have already been retrieved!");
	STINGRAYKIT_DECLARE_SIMPLE_EXCEPTION(CrcErrorException, "CRC mismatch!");
	STINGRAYKIT_DECLARE_SIMPLE_EXCEPTION(TimeoutException, "Timed out!");
	STINGRAYKIT_DECLARE_SIMPLE_EXCEPTION(InvalidOperationException, "Invalid operation!");
	STINGRAYKIT_DECLARE_SIMPLE_EXCEPTION(OperationCancelledException, "Operation has been cancelled!");
	STINGRAYKIT_DECLARE_SIMPLE_EXCEPTION(SocketException, "Socket error!");
	STINGRAYKIT_DECLARE_SIMPLE_EXCEPTION(AccessDeniedException, "Access denied!");


#define STINGRAYKIT_CHECK(Condition, ExceptionObj) \
		do { \
			if (STINGRAYKIT_UNLIKELY(!(Condition))) \
				STINGRAYKIT_THROW(ExceptionObj); \
		} while(false)

#define STINGRAYKIT_RETHROW_WITH_MESSAGE(Message, ExceptionObj) \
		do { \
			stingray::string_ostream stream; \
			stingray::ToString(stream, Message); \
			stream << ": "; \
			stingray::diagnostic_information(stream, ExceptionObj); \
			throw stingray::Exception(stream.str()); \
		} while (false)

#define STINGRAYKIT_WRAP_EXCEPTIONS(Message, ...) \
		do { \
			try { __VA_ARGS__; } \
			catch (const std::exception& ex) \
			{ STINGRAYKIT_RETHROW_WITH_MESSAGE(Message, ex); } \
		} while (false)


	struct LogicException : public std::logic_error
	{
		LogicException() : std::logic_error("You're doing something wrong!")
		{ DebuggingHelper::BreakpointHere(); }

		explicit LogicException(const std::string& message) : std::logic_error(message)
		{ DebuggingHelper::BreakpointHere(); }
	};

	struct ArgumentException : public Exception
	{
		ArgumentException() : Exception("Invalid argument!") { }
		explicit ArgumentException(const std::string& argName) : Exception("Invalid argument: " + argName) { }

		template < typename ArgumentType >
		ArgumentException(const std::string& argName, const ArgumentType& argValue) : Exception(BuildErrorMessage(argName, argValue)) { }

	private:
		template < typename ArgumentType >
		static std::string BuildErrorMessage(const std::string& argName, const ArgumentType& argValue)
		{
			string_ostream stream;
			stream << "Invalid argument '" << argName << "' value '";
			stingray::ToString(stream, argValue);
			stream << "'";
			return stream.str();
		}
	};

	struct NullArgumentException : public Exception
	{
		NullArgumentException() : Exception("Null argument!") { }
		explicit NullArgumentException(const std::string& argName) : Exception("Null argument: " + argName) { }
	};

	class IndexOutOfRangeException : public Exception
	{
	public:
		IndexOutOfRangeException() : Exception("Index out of range!") { }
		template < typename IndexType, typename SizeType >
		IndexOutOfRangeException(IndexType index, SizeType size) : Exception(BuildErrorMessage(index, size)) { }
		template < typename IndexType, typename LeftBoundaryType, typename RightBoundaryType >
		IndexOutOfRangeException(IndexType index, LeftBoundaryType begin, RightBoundaryType end) : Exception(BuildErrorMessage(index, begin, end)) { }

	private:
		template < typename IndexType, typename SizeType >
		static std::string BuildErrorMessage(IndexType index, SizeType size)
		{
			string_ostream stream;
			stream << "Index " << index << " out of range " << size;
			return stream.str();
		}

		template < typename IndexType, typename LeftBoundaryType, typename RightBoundaryType >
		static std::string BuildErrorMessage(IndexType index, LeftBoundaryType begin, RightBoundaryType end)
		{
			string_ostream stream;
			stream << "Index " << index << " out of range [" << begin << ", " << end << ")";
			return stream.str();
		}
	};

#define DETAIL_STINGRAYKIT_CHECK_RANGE_2(Index, Size) \
	STINGRAYKIT_CHECK((Index) < (Size), stingray::IndexOutOfRangeException(Index, Size))

#define DETAIL_STINGRAYKIT_CHECK_RANGE_3(Index, Begin, End) \
	STINGRAYKIT_CHECK((Index) >= (Begin) && (Index) < (End), stingray::IndexOutOfRangeException(Index, Begin, End))

#define STINGRAYKIT_CHECK_RANGE(...) \
	STINGRAYKIT_MACRODISPATCH(DETAIL_STINGRAYKIT_CHECK_RANGE_, __VA_ARGS__)


	struct FormatException : public Exception
	{
		FormatException() : Exception("Invalid format!") { }
		explicit FormatException(const std::string& expression) : Exception("Invalid format: " + expression) { }
	};

	struct MalformedDataException : public Exception
	{
		MalformedDataException() : Exception("Malformed data!") { }
		explicit MalformedDataException(const std::string& message) : Exception(message) { }
	};

	struct MalformedJsonException : public MalformedDataException
	{
		MalformedJsonException() : MalformedDataException("Malformed json!") { }
		explicit MalformedJsonException(const std::string& message) : MalformedDataException(message) { }
	};

	struct NullPointerException : public Exception
	{
		NullPointerException() : Exception("Accessing null pointer!")
		{ DebuggingHelper::BreakpointHere(); }
		explicit NullPointerException(const std::string& expr) : Exception("Accessing null pointer: " + expr)
		{ DebuggingHelper::BreakpointHere(); }
	};

	struct NotInitializedException : public Exception
	{
		NotInitializedException() : Exception("Accessing not initialized object!")
		{ DebuggingHelper::BreakpointHere(); }
		explicit NotInitializedException(const std::string& expr) : Exception("Accessing not initialized object: " + expr)
		{ DebuggingHelper::BreakpointHere(); }
	};

	struct InvalidCastException : public std::bad_cast
	{
	private:
		std::string _message;

	public:
		InvalidCastException() : _message("Invalid cast!") { }
		InvalidCastException(const std::string& source, const std::string& target) : _message(BuildErrorMessage(source, target)) { }
		virtual ~InvalidCastException() noexcept { }

		virtual const char* what() const noexcept { return _message.c_str(); }

	private:
		static std::string BuildErrorMessage(const std::string& source, const std::string& target)
		{
			string_ostream stream;
			stream << "Invalid cast from " << source << " to " << target;
			return stream.str();
		}
	};

	struct KeyNotFoundException : public Exception
	{
		KeyNotFoundException() : Exception("Key not found!") { }
		explicit KeyNotFoundException(const std::string& keyStr) : Exception(BuildErrorMessage(keyStr)) { }

	private:
		static std::string BuildErrorMessage(const std::string& keyStr)
		{
			string_ostream stream;
			stream << "Key '" << keyStr << "' not found!";
			return stream.str();
		}
	};

	struct FileNotFoundException : public Exception
	{
		FileNotFoundException() : Exception("File not found!") { }
		explicit FileNotFoundException(const std::string& path) : Exception(BuildErrorMessage(path)) { }

	private:
		static std::string BuildErrorMessage(const std::string& path)
		{
			string_ostream stream;
			stream << "File '" << path << "' not found!";
			return stream.str();
		}
	};
	STINGRAYKIT_DECLARE_SIMPLE_EXCEPTION(NoSpaceLeftException, "No space left on device!");
	STINGRAYKIT_DECLARE_SIMPLE_EXCEPTION(InputOutputException, "Input/output error on storage!");


	namespace Detail
	{

		struct IToolkitException
		{
			virtual ~IToolkitException() noexcept { }

			virtual size_t GetLine() const = 0;
			virtual const char* GetFilename() const = 0;
			virtual const char* GetFunctionName() const = 0;
			virtual std::string GetBacktrace() const = 0;
		};

	}

	class BaseException : public virtual Detail::IToolkitException
	{
	private:
		ToolkitWhere	_where;
		Backtrace		_backtrace;

	protected:
		explicit BaseException(ToolkitWhere where) : _where(where)
		{ }

	public:
		virtual ~BaseException() noexcept { }

		virtual size_t GetLine() const				{ return _where.GetLine(); }
		virtual const char* GetFilename() const		{ return _where.GetFilename(); }
		virtual const char* GetFunctionName() const	{ return _where.GetFunctionName(); }
		virtual std::string GetBacktrace() const	{ return _backtrace.Get(); }
	};

	template < typename UserBaseException >
	class ExceptionWrapper : public BaseException, public UserBaseException
	{
	public:
		ExceptionWrapper(const UserBaseException& ex, ToolkitWhere where)
			: BaseException(where), UserBaseException(ex)
		{ }
	};


	namespace Detail
	{
		template < typename BaseException >
		inline typename EnableIf<IsInherited<BaseException, std::exception>::Value, ExceptionWrapper<BaseException> >::ValueT MakeException(const BaseException& ex, ToolkitWhere where)
		{
			static_assert(IsInherited<BaseException, std::exception>::Value, "Invalid exception type");
			return ExceptionWrapper<BaseException>(ex, where);
		}

		inline ExceptionWrapper<Exception> MakeException(const std::string& message, ToolkitWhere where)
		{ return MakeException(Exception(message), where); }

		inline ExceptionWrapper<Exception> MakeException(const char *message, ToolkitWhere where)
		{ return MakeException(Exception(message), where); }


		template < typename T >
		T&& RequireNotNull(T&& obj, const char* expr, ToolkitWhere where)
		{
			if (obj)
				return std::forward<T>(obj);

			DebuggingHelper::BreakpointHere();
			throw MakeException(NullPointerException(expr), where);
		}

		template < typename T >
		T&& RequireInitialized(T&& obj, const char* expr, ToolkitWhere where)
		{
			if (obj)
				return std::forward<T>(obj);

			DebuggingHelper::BreakpointHere();
			throw MakeException(NotInitializedException(expr), where);
		}
	}


#define STINGRAYKIT_MAKE_EXCEPTION(...) ::stingray::Detail::MakeException(__VA_ARGS__, STINGRAYKIT_WHERE)
#define STINGRAYKIT_THROW(...) throw ::stingray::Detail::MakeException(__VA_ARGS__, STINGRAYKIT_WHERE)

#define STINGRAYKIT_REQUIRE_NOT_NULL(Expr_) stingray::Detail::RequireNotNull(Expr_, #Expr_, STINGRAYKIT_WHERE)
#define STINGRAYKIT_REQUIRE_INITIALIZED(Expr_) stingray::Detail::RequireInitialized(Expr_, #Expr_, STINGRAYKIT_WHERE)


	void _append_extended_diagnostics(string_ostream& result, const Detail::IToolkitException& tkit_ex);

	template < typename ExceptionType >
	inline void diagnostic_information(string_ostream& result, const ExceptionType& ex)
	{
		const Detail::IToolkitException* tkit_ex = dynamic_cast<const Detail::IToolkitException*>(&ex);
		const std::exception* std_ex = dynamic_cast<const std::exception*>(&ex);
		const std::type_info& ex_ti = typeid(ex);

		if (std_ex)
			result << Demangle(ex_ti.name()) << "\n" << std_ex->what();
		else
			result << "Unknown exception: " << Demangle(ex_ti.name());

		if (tkit_ex)
			_append_extended_diagnostics(result, *tkit_ex);
	}

	template < typename ExceptionType >
	inline std::string diagnostic_information(const ExceptionType& ex)
	{
		string_ostream result;
		diagnostic_information(result, ex);
		return result.str();
	}

}

#endif
