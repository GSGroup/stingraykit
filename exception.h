#ifndef __GS_DVRLIB_TOOLKIT_EXCEPTION_H__
#define __GS_DVRLIB_TOOLKIT_EXCEPTION_H__


#include <stdexcept>
#include <typeinfo>
#include <string>


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

	/*! \cond GS_INTERNAL */

	template < typename T >
	class shared_ptr;

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
	TOOLKIT_DECLARE_SIMPLE_EXCEPTION(IndexOutOfRangeException, "Index out of range!");
	TOOLKIT_DECLARE_SIMPLE_EXCEPTION(FormatException, "Invalid format!");
	TOOLKIT_DECLARE_SIMPLE_EXCEPTION(DeviceBusyException, "Device is busy!");
	TOOLKIT_DECLARE_SIMPLE_EXCEPTION(BrokenPromise, "Promise destroyed before value is set!");
	TOOLKIT_DECLARE_SIMPLE_EXCEPTION(PromiseAlreadySatisfied, "Promise value have already been set!");
	TOOLKIT_DECLARE_SIMPLE_EXCEPTION(FutureAlreadyRetrieved, "Future have already been retrieved!");
	TOOLKIT_DECLARE_SIMPLE_EXCEPTION(CrcErrorException, "CRC mismatch!");
	TOOLKIT_DECLARE_SIMPLE_EXCEPTION(TimeoutException, "Timed out!");

#define TOOLKIT_CHECK(Condition, ExceptionObj) \
		do { if (!(Condition)) TOOLKIT_THROW(ExceptionObj); } while(false)

#define TOOLKIT_INDEX_CHECK(...) \
		TOOLKIT_CHECK((__VA_ARGS__), IndexOutOfRangeException())

#define TOOLKIT_NOT_IMPLEMENTED() \
		TOOLKIT_THROW(NotImplementedException())

	struct ArgumentException : public std::runtime_error
	{
		ArgumentException() : std::runtime_error("Invalid argument!") { }
		ArgumentException(const std::string& argName) : std::runtime_error("Invalid argument: " + argName) { }
	};

	struct NullPointerException : public std::runtime_error
	{
		NullPointerException() : std::runtime_error("Accessing null pointer!") { }
		NullPointerException(const std::string& expr) : std::runtime_error("Accessing null pointer: " + expr) { }
	};

	struct InvalidCastException : public std::runtime_error
	{
		InvalidCastException() : std::runtime_error("Invalid cast!") { }
		InvalidCastException(const std::string& source, const std::string& target) : std::runtime_error("Invalid cast from " + source + " to " + target) { }
	};


#define TOOLKIT_REQUIRE_NOT_NULL(Expr_) stingray::Detail::RequireNotNull(Expr_, #Expr_, __FILE__, __LINE__, TOOLKIT_FUNCTION)


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
		size_t			_line;
		const char*		_filename;
		const char*		_functionName;
#ifdef USE_BACKTRACE_FOR_EXCEPTIONS
		Backtrace		_backtrace;
#endif

	public:
		BaseException(const char* filename, size_t line, const char* functionName)
			: _line(line), _filename(filename), _functionName(functionName)
		{}

		virtual ~BaseException() throw() { }

		virtual size_t GetLine() const				{ return _line; }
		virtual const char* GetFilename() const		{ return _filename; }
		virtual const char* GetFunctionName() const	{ return _functionName; }
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
		ExceptionWrapper(const UserBaseException& ex, const char* filename, size_t line, const char* functionName)
			: BaseException(filename, line, functionName), UserBaseException(ex)
		{ }
		virtual ~ExceptionWrapper() throw() { }
	};


	namespace Detail
	{
		template < typename BaseException >
		inline ExceptionWrapper<BaseException> MakeException(const BaseException& ex, const char* filename, size_t line, const char* functionName)
		{
			CompileTimeAssert<Inherits<BaseException, std::exception>::Value > ERROR_invalid_exception;
			(void)ERROR_invalid_exception;
			return ExceptionWrapper<BaseException>(ex, filename, line, functionName);
		}

		inline ExceptionWrapper<Exception> MakeException(const std::string &message, const char* filename, size_t line, const char* functionName)
		{
			return MakeException(Exception(message), filename, line, functionName);
		}

		inline ExceptionWrapper<Exception> MakeException(const char *message, const char* filename, size_t line, const char* functionName)
		{
			return MakeException(Exception(message), filename, line, functionName);
		}

		template < typename T, bool CanCastToBool = CanCast<T, bool>::Value >
		struct NullTester
		{ static bool Test(const T& val)		{ return true; } };

		template < typename T >
		struct NullTester<T, true>
		{ static bool Test(const T& val)		{ return val; } };

		template < typename T >
		bool TestNull(const T& val) { return NullTester<T>::Test(val); }

		/*
		template < typename T >
		T& RequireNotNull(T& obj, const char* expr, const char* file, size_t line, const char* func)
		{ if (!TestNull(obj)) throw stingray::Detail::MakeException(NullPointerException(expr), file, line, func); else return obj; }
		*/

		template < typename T >
		const T& RequireNotNull(const T& obj, const char* expr, const char* file, size_t line, const char* func, int dummy = 42)
		{
			if (!TestNull(obj))
			{
				DebuggingHelper::BreakpointHere();
				throw stingray::Detail::MakeException(NullPointerException(expr), file, line, func);
			}
			else
				return obj;
		}
	}


#define TOOLKIT_THROW(ExceptionObj) throw ::stingray::Detail::MakeException(ExceptionObj, __FILE__, __LINE__, TOOLKIT_FUNCTION)

	void _append_extended_diagnostics(std::stringstream& result, const Detail::IToolkitException& tkit_ex);

	template < typename ExceptionType >
	inline std::string diagnostic_information(const ExceptionType& ex)
	{
		const Detail::IToolkitException* tkit_ex = dynamic_cast<const Detail::IToolkitException*>(&ex);
		const std::exception* std_ex = dynamic_cast<const std::exception*>(&ex);
		const std::type_info& ex_ti = typeid(ex);
		std::stringstream result;

		if (std_ex)
			result << "std::exception: " << ex_ti.name() << "\n" << std_ex->what();
		else
			result << "Unknown exception: " << ex_ti.name();

		if (tkit_ex)
			_append_extended_diagnostics(result, *tkit_ex);

		return result.str();
	}

	/*! \endcond */

}


#endif
