#ifndef STINGRAYKIT_EXCEPTION_H
#define STINGRAYKIT_EXCEPTION_H

// Copyright (c) 2011 - 2024, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/diagnostics/Backtrace.h>
#include <stingraykit/string/ToStringForward.h>

#include <stdexcept>
#include <typeinfo>

#define STINGRAYKIT_DECLARE_SIMPLE_EXCEPTION(ExceptionClass, Message) \
	struct ExceptionClass : public stingray::Exception \
	{ \
		ExceptionClass() : stingray::Exception(Message) { } \
		explicit ExceptionClass(const std::string& message) : stingray::Exception(Message, message) { } \
		explicit ExceptionClass(string_view message) : stingray::Exception(Message, message) { } \
		explicit ExceptionClass(const char* message) : stingray::Exception(Message, message) { } \
	}

namespace stingray
{

	class Exception : public std::runtime_error
	{
	public:
		explicit Exception(const std::string& message) : std::runtime_error(message) { }
		explicit Exception(string_view message) : std::runtime_error(message.copy()) { }
		explicit Exception(const char* message) : std::runtime_error(message) { }

	protected:
		Exception(string_view message, string_view additionalMessage) : Exception(BuildErrorMessage(message, additionalMessage)) { }

	private:
		static std::string BuildErrorMessage(string_view message, string_view additionalMessage)
		{
			string_ostream stream;
			stream << message << ": " << additionalMessage;
			return stream.str();
		}
	};

	STINGRAYKIT_DECLARE_SIMPLE_EXCEPTION(NotImplementedException, "The feature is not implemented");
	STINGRAYKIT_DECLARE_SIMPLE_EXCEPTION(NotSupportedException, "The feature is not supported");
	STINGRAYKIT_DECLARE_SIMPLE_EXCEPTION(DeviceBusyException, "Device is busy");
	STINGRAYKIT_DECLARE_SIMPLE_EXCEPTION(ResourceUnavailableException, "Resource temporarily unavailable");
	STINGRAYKIT_DECLARE_SIMPLE_EXCEPTION(BrokenPromise, "Promise destroyed before value is set");
	STINGRAYKIT_DECLARE_SIMPLE_EXCEPTION(PromiseAlreadySatisfied, "Promise value have already been set");
	STINGRAYKIT_DECLARE_SIMPLE_EXCEPTION(FutureAlreadyRetrieved, "Future have already been retrieved");
	STINGRAYKIT_DECLARE_SIMPLE_EXCEPTION(CrcErrorException, "CRC mismatch");
	STINGRAYKIT_DECLARE_SIMPLE_EXCEPTION(TimeoutException, "Timed out");
	STINGRAYKIT_DECLARE_SIMPLE_EXCEPTION(InvalidOperationException, "Invalid operation");
	STINGRAYKIT_DECLARE_SIMPLE_EXCEPTION(OperationCancelledException, "Operation has been cancelled");
	STINGRAYKIT_DECLARE_SIMPLE_EXCEPTION(SocketException, "Socket error");
	STINGRAYKIT_DECLARE_SIMPLE_EXCEPTION(AccessDeniedException, "Access denied");


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
		LogicException() : std::logic_error("You're doing something wrong")
		{ DebuggingHelper::BreakpointHere(); }
		explicit LogicException(const std::string& message) : std::logic_error(message)
		{ DebuggingHelper::BreakpointHere(); }
		explicit LogicException(string_view message) : std::logic_error(message.copy())
		{ DebuggingHelper::BreakpointHere(); }
		explicit LogicException(const char* message) : std::logic_error(message)
		{ DebuggingHelper::BreakpointHere(); }
	};

	struct ArgumentException : public Exception
	{
		ArgumentException() : Exception("Invalid argument") { }
		explicit ArgumentException(const std::string& argName) : Exception("Invalid argument", argName) { }
		explicit ArgumentException(string_view argName) : Exception("Invalid argument", argName) { }
		explicit ArgumentException(const char* argName) : Exception("Invalid argument", argName) { }

		template < typename ArgumentType >
		ArgumentException(const std::string& argName, const ArgumentType& argValue) : Exception(BuildErrorMessage(argName, argValue)) { }
		template < typename ArgumentType >
		ArgumentException(string_view argName, const ArgumentType& argValue) : Exception(BuildErrorMessage(argName, argValue)) { }
		template < typename ArgumentType >
		ArgumentException(const char* argName, const ArgumentType& argValue) : Exception(BuildErrorMessage(argName, argValue)) { }

	private:
		template < typename ArgumentType >
		static std::string BuildErrorMessage(string_view argName, const ArgumentType& argValue)
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
		NullArgumentException() : Exception("Null argument") { }
		explicit NullArgumentException(const std::string& argName) : Exception("Null argument", argName) { }
		explicit NullArgumentException(string_view argName) : Exception("Null argument", argName) { }
		explicit NullArgumentException(const char* argName) : Exception("Null argument", argName) { }
	};

	class IndexOutOfRangeException : public Exception
	{
	public:
		IndexOutOfRangeException() : Exception("Index out of range") { }
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


	class IntegerOverflowException : public Exception
	{
	public:
		IntegerOverflowException() : Exception("Integer overflow") { }
		template < typename IntegerType, typename IncrementType >
		IntegerOverflowException(IntegerType value, IncrementType increment) : Exception(BuildErrorMessage(value, increment)) { }

	private:
		template < typename IntegerType, typename IncrementType >
		static std::string BuildErrorMessage(IntegerType value, IncrementType increment)
		{
			string_ostream stream;
			stream << "Integer value (" << value << " + " << increment << ") will ";
			if (increment >= 0)
				stream << "overflow " << std::numeric_limits<IntegerType>::max();
			else
				stream << "underflow " << std::numeric_limits<IntegerType>::min();
			return stream.str();
		}
	};

	struct FormatException : public Exception
	{
		FormatException() : Exception("Invalid format") { }
		explicit FormatException(const std::string& expression) : Exception(BuildErrorMessage(expression)) { }
		explicit FormatException(string_view expression) : Exception(BuildErrorMessage(expression)) { }
		explicit FormatException(const char* expression) : Exception(BuildErrorMessage(expression)) { }

	private:
		static std::string BuildErrorMessage(string_view expression)
		{
			string_ostream stream;
			stream << "Invalid format: '" << expression << "'";
			return stream.str();
		}
	};

	struct MalformedDataException : public Exception
	{
		MalformedDataException() : Exception("Malformed data") { }
		explicit MalformedDataException(const std::string& message) : Exception(message) { }
		explicit MalformedDataException(string_view message) : Exception(message) { }
		explicit MalformedDataException(const char* message) : Exception(message) { }
	};

	struct MalformedJsonException : public MalformedDataException
	{
		MalformedJsonException() : MalformedDataException("Malformed json") { }
		explicit MalformedJsonException(const std::string& message) : MalformedDataException(message) { }
		explicit MalformedJsonException(const string_view message) : MalformedDataException(message) { }
		explicit MalformedJsonException(const char* message) : MalformedDataException(message) { }
	};

	struct NullPointerException : public Exception
	{
		NullPointerException() : Exception("Accessing null pointer")
		{ DebuggingHelper::BreakpointHere(); }
		explicit NullPointerException(const std::string& expr) : Exception("Accessing null pointer", expr)
		{ DebuggingHelper::BreakpointHere(); }
		explicit NullPointerException(string_view expr) : Exception("Accessing null pointer", expr)
		{ DebuggingHelper::BreakpointHere(); }
		explicit NullPointerException(const char* expr) : Exception("Accessing null pointer", expr)
		{ DebuggingHelper::BreakpointHere(); }
	};

	struct NotInitializedException : public Exception
	{
		NotInitializedException() : Exception("Accessing not initialized object")
		{ DebuggingHelper::BreakpointHere(); }
		explicit NotInitializedException(const std::string& expr) : Exception("Accessing not initialized object", expr)
		{ DebuggingHelper::BreakpointHere(); }
		explicit NotInitializedException(string_view expr) : Exception("Accessing not initialized object", expr)
		{ DebuggingHelper::BreakpointHere(); }
		explicit NotInitializedException(const char* expr) : Exception("Accessing not initialized object", expr)
		{ DebuggingHelper::BreakpointHere(); }
	};

	struct InvalidCastException : public std::bad_cast
	{
	private:
		std::string			_message;

	public:
		InvalidCastException() : _message("Invalid cast") { }
		InvalidCastException(const std::string& source, const std::string& target) : _message(BuildErrorMessage(source, target)) { }
		InvalidCastException(string_view source, string_view target) : _message(BuildErrorMessage(source, target)) { }
		InvalidCastException(const char* source, const char* target) : _message(BuildErrorMessage(source, target)) { }
		~InvalidCastException() noexcept override { }

		const char* what() const noexcept override { return _message.c_str(); }

	private:
		static std::string BuildErrorMessage(string_view source, string_view target)
		{
			string_ostream stream;
			stream << "Invalid cast from " << source << " to " << target;
			return stream.str();
		}
	};

	struct KeyNotFoundException : public Exception
	{
		KeyNotFoundException() : Exception("Key not found") { }
		explicit KeyNotFoundException(const std::string& keyStr) : Exception(BuildErrorMessage(keyStr)) { }
		explicit KeyNotFoundException(string_view keyStr) : Exception(BuildErrorMessage(keyStr)) { }
		explicit KeyNotFoundException(const char* keyStr) : Exception(BuildErrorMessage(keyStr)) { }

	private:
		static std::string BuildErrorMessage(string_view keyStr)
		{
			string_ostream stream;
			stream << "Key '" << keyStr << "' not found";
			return stream.str();
		}
	};

	struct FileNotFoundException : public Exception
	{
		FileNotFoundException() : Exception("File not found") { }
		explicit FileNotFoundException(const std::string& path) : Exception(BuildErrorMessage(path)) { }
		explicit FileNotFoundException(string_view path) : Exception(BuildErrorMessage(path)) { }
		explicit FileNotFoundException(const char* path) : Exception(BuildErrorMessage(path)) { }

	private:
		static std::string BuildErrorMessage(string_view path)
		{
			string_ostream stream;
			stream << "File '" << path << "' not found";
			return stream.str();
		}
	};
	STINGRAYKIT_DECLARE_SIMPLE_EXCEPTION(NoSpaceLeftException, "No space left on device");
	STINGRAYKIT_DECLARE_SIMPLE_EXCEPTION(InputOutputException, "Input/output error on storage");


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
		~BaseException() noexcept override { }

		size_t GetLine() const override					{ return _where.GetLine(); }
		const char* GetFilename() const override		{ return _where.GetFilename(); }
		const char* GetFunctionName() const override	{ return _where.GetFunctionName(); }
		std::string GetBacktrace() const override		{ return _backtrace.Get(); }
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
		typename EnableIf<IsInherited<BaseException, std::exception>::Value, ExceptionWrapper<BaseException> >::ValueT MakeException(const BaseException& ex, ToolkitWhere where)
		{
			static_assert(IsInherited<BaseException, std::exception>::Value, "Invalid exception type");
			return ExceptionWrapper<BaseException>(ex, where);
		}

		inline ExceptionWrapper<Exception> MakeException(const std::string& message, ToolkitWhere where)
		{ return MakeException(Exception(message), where); }

		inline ExceptionWrapper<Exception> MakeException(string_view message, ToolkitWhere where)
		{ return MakeException(Exception(message), where); }

		inline ExceptionWrapper<Exception> MakeException(const char *message, ToolkitWhere where)
		{ return MakeException(Exception(message), where); }


		template < typename IntegerType, typename IncrementType >
		void CheckIntegerOverflow(IntegerType value, IncrementType increment, ToolkitWhere where)
		{
			constexpr auto valueMax = std::numeric_limits<typename std::make_unsigned<IntegerType>::type>::max();
			constexpr auto incrementMax = std::numeric_limits<typename std::make_unsigned<IncrementType>::type>::max();

			static_assert(IsSame<IntegerType, IncrementType>::Value
							|| (std::is_unsigned<IntegerType>::value && valueMax >= incrementMax)
							|| (std::is_signed<IntegerType>::value && std::is_unsigned<IncrementType>::value && valueMax > incrementMax)
							|| (std::is_signed<IntegerType>::value && std::is_signed<IncrementType>::value && valueMax >= incrementMax),
					"Increment must have same or lesser type than value");

			if (increment > 0)
			{
				if (value > std::numeric_limits<IntegerType>::max() - increment)
					throw MakeException(IntegerOverflowException(value, increment), where);
			}
			else if (increment < 0 && value < std::numeric_limits<IntegerType>::min() - increment)
				throw MakeException(IntegerOverflowException(value, increment), where);
		}


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


		void AppendExtendedDiagnostics(string_ostream& result, const Detail::IToolkitException& tkit_ex);
	}


#define STINGRAYKIT_MAKE_EXCEPTION(...) ::stingray::Detail::MakeException(__VA_ARGS__, STINGRAYKIT_WHERE)
#define STINGRAYKIT_THROW(...) throw ::stingray::Detail::MakeException(__VA_ARGS__, STINGRAYKIT_WHERE)

#define STINGRAYKIT_CHECK_INTEGER_OVERFLOW(Value, Increment) stingray::Detail::CheckIntegerOverflow(Value, Increment, STINGRAYKIT_WHERE)

#define STINGRAYKIT_REQUIRE_NOT_NULL(Expr_) stingray::Detail::RequireNotNull(Expr_, #Expr_, STINGRAYKIT_WHERE)
#define STINGRAYKIT_REQUIRE_INITIALIZED(Expr_) stingray::Detail::RequireInitialized(Expr_, #Expr_, STINGRAYKIT_WHERE)


	template < typename ExceptionType >
	void diagnostic_information(string_ostream& result, const ExceptionType& ex)
	{
		const Detail::IToolkitException* tkit_ex = dynamic_cast<const Detail::IToolkitException*>(&ex);
		const std::exception* std_ex = dynamic_cast<const std::exception*>(&ex);
		const std::string& exName = Demangle(typeid(ex).name());

		if (std_ex)
			result << exName << "\n" << std_ex->what();
		else
			result << "Unknown exception: " << exName;

		if (tkit_ex)
			Detail::AppendExtendedDiagnostics(result, *tkit_ex);
	}

	template < typename ExceptionType >
	std::string diagnostic_information(const ExceptionType& ex)
	{
		string_ostream result;
		diagnostic_information(result, ex);
		return result.str();
	}

}

#endif
