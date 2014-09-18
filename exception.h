#ifndef STINGRAY_TOOLKIT_EXCEPTION_H
#define STINGRAY_TOOLKIT_EXCEPTION_H


#include <stdexcept>
#include <typeinfo>
#include <string>
#include <stingray/toolkit/string_stream.h>


#ifdef USE_BACKTRACE_FOR_EXCEPTIONS
#	include <stingray/toolkit/Backtrace.h>
#endif

#include <stingray/toolkit/MetaProgramming.h>
#include <stingray/toolkit/toolkit.h>

#define TOOLKIT_DECLARE_SIMPLE_EXCEPTION(ExceptionClass, Message) \
	struct ExceptionClass : public stingray::Exception \
	{ \
		ExceptionClass() : stingray::Exception(Message) { } \
		virtual ~ExceptionClass() throw() { } \
	}


namespace stingray
{

	template < typename T >
	std::string ToString(const T& val); // for ArgumentException; TODO find better workaround

	class Exception : public std::runtime_error
	{
	public:
		Exception(const std::string& message)
			: std::runtime_error(message)
		{ }
		virtual ~Exception() throw() { }
	};

	TOOLKIT_DECLARE_SIMPLE_EXCEPTION(NotImplementedException, "The feature is not implemented!");
	TOOLKIT_DECLARE_SIMPLE_EXCEPTION(NotSupportedException, "The feature is not supported!");
	TOOLKIT_DECLARE_SIMPLE_EXCEPTION(DeviceBusyException, "Device is busy!");
	TOOLKIT_DECLARE_SIMPLE_EXCEPTION(BrokenPromise, "Promise destroyed before value is set!");
	TOOLKIT_DECLARE_SIMPLE_EXCEPTION(PromiseAlreadySatisfied, "Promise value have already been set!");
	TOOLKIT_DECLARE_SIMPLE_EXCEPTION(FutureAlreadyRetrieved, "Future have already been retrieved!");
	TOOLKIT_DECLARE_SIMPLE_EXCEPTION(CrcErrorException, "CRC mismatch!");
	TOOLKIT_DECLARE_SIMPLE_EXCEPTION(TimeoutException, "Timed out!");
	TOOLKIT_DECLARE_SIMPLE_EXCEPTION(InvalidOperationException, "Invalid operation!");
	TOOLKIT_DECLARE_SIMPLE_EXCEPTION(OperationCanceledException, "Operation has been canceled!");
	TOOLKIT_DECLARE_SIMPLE_EXCEPTION(SocketException, "Socket error!");

#define TOOLKIT_CHECK(Condition, ExceptionObj) \
		do { if (!(Condition)) TOOLKIT_THROW(ExceptionObj); } while(false)

	struct LogicException : public std::logic_error
	{
		LogicException() : std::logic_error("You're doing something wrong!")
		{ DebuggingHelper::BreakpointHere(); }

		LogicException(const std::string& message) : std::logic_error(message)
		{ DebuggingHelper::BreakpointHere(); }
	};

	struct ArgumentException : public Exception
	{
		ArgumentException() : Exception("Invalid argument!") { }
		ArgumentException(const std::string& argName) : Exception("Invalid argument: " + argName) { }
		template < typename ArgumentType >
		ArgumentException(const std::string& argName, const ArgumentType& argValue) : Exception("Invalid argument '" + argName + "' value '" + ToString(argValue) + "'") { }
	};

	struct NullArgumentException : public Exception
	{
		NullArgumentException() : Exception("Null argument!") { }
		NullArgumentException(const std::string& argName) : Exception("Null argument: " + argName) { }
	};

	class IndexOutOfRangeException : public Exception
	{
	public:
		IndexOutOfRangeException() : Exception("Index out of range!") { }
		IndexOutOfRangeException(size_t index, size_t size) : Exception(BuildErrorMessage(index, size)) { }

	private:
		static std::string BuildErrorMessage(size_t index, size_t size)
		{
			string_ostream stream;
			stream << "Index " << index << " out of range " << size;
			return stream.str();
		}
	};

	struct FormatException : public Exception
	{
		FormatException() : Exception("Invalid format!") { }
		FormatException(const std::string& expression) : Exception("Invalid format: " + expression) { }
	};

	struct MalformedDataException : public Exception
	{
		MalformedDataException() : Exception("Malformed data!") { }
		MalformedDataException(const std::string& expression) : Exception("Malformed data: " + expression) { }
	};

	struct NullPointerException : public Exception
	{
		NullPointerException() : Exception("Accessing null pointer!") { }
		NullPointerException(const std::string& expr) : Exception("Accessing null pointer: " + expr) { }
	};

	struct InvalidCastException : public std::bad_cast
	{
	private:
		std::string _message;

	public:
		InvalidCastException() : _message("Invalid cast!") { }
		InvalidCastException(const std::string& source, const std::string& target) : _message("Invalid cast from " + source + " to " + target) { }
		virtual ~InvalidCastException() throw() { }

		virtual const char* what() const throw() { return _message.c_str(); }
	};

	struct KeyNotFoundException : public Exception
	{
		KeyNotFoundException() : Exception("Key not found!") { }
		KeyNotFoundException(const std::string& keyStr) : Exception("Key '" + keyStr + "' not found!") { }
		virtual ~KeyNotFoundException() throw() { }
	};


#define TOOLKIT_REQUIRE_NOT_NULL(Expr_) stingray::Detail::RequireNotNull(Expr_, #Expr_, TOOLKIT_WHERE)


	namespace Detail
	{

		struct IToolkitException
		{
			virtual ~IToolkitException() throw() { }

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
#ifdef USE_BACKTRACE_FOR_EXCEPTIONS
		Backtrace		_backtrace;
#endif

	public:
		BaseException(ToolkitWhere where)
			: _where(where)
		{}

		virtual ~BaseException() throw() { }

		virtual size_t GetLine() const				{ return _where.GetLine(); }
		virtual const char* GetFilename() const		{ return _where.GetFilename(); }
		virtual const char* GetFunctionName() const	{ return _where.GetFunctionName(); }
#ifdef USE_BACKTRACE_FOR_EXCEPTIONS
		virtual std::string GetBacktrace() const	{ return _backtrace.Get(); }
#else
		virtual std::string GetBacktrace() const	{ return std::string(); }
#endif
	};

	template < typename UserBaseException >
	class ExceptionWrapper : public BaseException, public UserBaseException
	{
	public:
		ExceptionWrapper(const UserBaseException& ex, ToolkitWhere where)
			: BaseException(where), UserBaseException(ex)
		{ }
		virtual ~ExceptionWrapper() throw() { }
	};


	namespace Detail
	{
		template < typename BaseException >
		inline typename EnableIf<Inherits<BaseException, std::exception>::Value, ExceptionWrapper<BaseException> >::ValueT MakeException(const BaseException& ex, ToolkitWhere where)
		{
			CompileTimeAssert<Inherits<BaseException, std::exception>::Value > ERROR_invalid_exception;
			(void)ERROR_invalid_exception;
			return ExceptionWrapper<BaseException>(ex, where);
		}

		inline ExceptionWrapper<Exception> MakeException(const std::string &message, ToolkitWhere where)
		{
			return MakeException(Exception(message), where);
		}

		inline ExceptionWrapper<Exception> MakeException(const char *message, ToolkitWhere where)
		{
			return MakeException(Exception(message), where);
		}

		template < typename T, bool CanCastToBool = CanCast<T, bool>::Value >
		struct NullTester;

		template < typename T >
		struct NullTester<T, true>
		{ static bool Test(const T& val) { return val; } };

		template < typename T >
		bool IsNotNull(const T& val) { return NullTester<T>::Test(val); }

		/*
		template < typename T >
		T& RequireNotNull(T& obj, const char* expr, const char* file, size_t line, const char* func)
		{ if (!IsNotNull(obj)) throw stingray::Detail::MakeException(NullPointerException(expr), file, line, func); else return obj; }
		*/

		template < typename T >
		const T& RequireNotNull(const T& obj, const char* expr, ToolkitWhere where, int dummy = 42)
		{
			if (IsNotNull(obj))
				return obj;

			DebuggingHelper::BreakpointHere();
			throw stingray::Detail::MakeException(NullPointerException(expr), where);
		}
	}


#define TOOLKIT_MAKE_EXCEPTION(...) ::stingray::Detail::MakeException(__VA_ARGS__, TOOLKIT_WHERE)
#define TOOLKIT_THROW(...) throw ::stingray::Detail::MakeException(__VA_ARGS__, TOOLKIT_WHERE)

	void _append_extended_diagnostics(string_ostream& result, const Detail::IToolkitException& tkit_ex);

	template < typename ExceptionType >
	inline void diagnostic_information(string_ostream & result, const ExceptionType& ex)
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
