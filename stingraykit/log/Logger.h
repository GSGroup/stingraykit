#ifndef STINGRAYKIT_LOG_LOGGER_H
#define STINGRAYKIT_LOG_LOGGER_H

// Copyright (c) 2011 - 2019, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/log/ILoggerSink.h>
#include <stingraykit/log/LoggerStream.h>
#include <stingraykit/time/ElapsedTime.h>
#include <stingraykit/Token.h>

#include <set>

namespace stingray
{

	/**
	 * @addtogroup toolkit_log
	 * @{
	 */

#define DETAIL_DEFINE_NAMED_LOGGER_1(ClassName) stingray::NamedLogger ClassName::s_logger(#ClassName)
#define DETAIL_DEFINE_NAMED_LOGGER_2(ClassName, LoggerName) stingray::NamedLogger ClassName::s_logger(LoggerName)

#define STINGRAYKIT_DEFINE_NAMED_LOGGER(...) STINGRAYKIT_CAT(DETAIL_DEFINE_NAMED_LOGGER_, STINGRAYKIT_NARGS(__VA_ARGS__))(__VA_ARGS__)


#define STINGRAYKIT_TRY(ErrorMessage_, ...) \
		do { \
			DETAIL_DECLARE_STATIC_LOGGER_ACCESSOR; \
			try { __VA_ARGS__; } \
			catch (const std::exception& ex) \
			{ \
				try { STINGRAYKIT_STATIC_LOGGER.Warning() << (ErrorMessage_) << ":\n" << ex; } \
				catch (const std::exception& ex) \
				{ STINGRAYKIT_STATIC_LOGGER.Warning() << "Can't print error message:\n" << STINGRAYKIT_WHERE << "\n" << ex; } \
			} \
		} while (0)


#define STINGRAYKIT_TRY_EX(LogLevel_, ErrorMessage_, ...) \
		do { \
			DETAIL_DECLARE_STATIC_LOGGER_ACCESSOR; \
			try { __VA_ARGS__; } \
			catch (const std::exception& ex) \
			{ \
				try { STINGRAYKIT_STATIC_LOGGER.LogLevel_() << (ErrorMessage_) << ":\n" << ex; } \
				catch (const std::exception& ex) \
				{ STINGRAYKIT_STATIC_LOGGER.LogLevel_() << "Can't print error message:\n" << STINGRAYKIT_WHERE << "\n" << ex; } \
			} \
		} while (0)


#define STINGRAYKIT_TRY_NO_MESSAGE(...) STINGRAYKIT_TRY(#__VA_ARGS__, __VA_ARGS__)


#define STINGRAYKIT_LOG_EXCEPTIONS(ErrorMessage_, ...) \
		do { \
			DETAIL_DECLARE_STATIC_LOGGER_ACCESSOR; \
			try { __VA_ARGS__; } \
			catch (const std::exception& ex) { STINGRAYKIT_STATIC_LOGGER.Warning() << (ErrorMessage_) << ":\n" << ex; throw; } \
		} while (0)


	class Logger
	{
		friend class NamedLogger;

	public:
		static LoggerStream Stream(LogLevel logLevel, DuplicatingLogsFilter* duplicatingLogsFilter = NULL);

		static LoggerStream Trace()		{ return Stream(LogLevel::Trace); }
		static LoggerStream Debug()		{ return Stream(LogLevel::Debug); }
		static LoggerStream Info()		{ return Stream(LogLevel::Info); }
		static LoggerStream Warning()	{ return Stream(LogLevel::Warning); }
		static LoggerStream Error()		{ return Stream(LogLevel::Error); }

		static void SetLogLevel(LogLevel logLevel);
		static LogLevel GetLogLevel();

		/// @name Named loggers control
		/// @{
		static void SetLogLevel(const std::string& loggerName, optional<LogLevel> logLevel);
		static void EnableBacktrace(const std::string& loggerName, bool enable);
		static void EnableHighlight(const std::string& loggerName, bool enable);

		static std::set<std::string> GetLoggerNames();
		/// @}

		static Token AddSink(const ILoggerSinkPtr& sink);

	private:
		static void DoLog(const NamedLoggerParams* namedLogger, LogLevel logLevel, const std::string& message);
	};


	class NamedLogger
	{
		struct OptionalLogLevel
		{
			STINGRAYKIT_ENUM_VALUES
			(
				Trace	= LogLevel::Trace,
				Debug	= LogLevel::Debug,
				Info	= LogLevel::Info,
				Warning	= LogLevel::Warning,
				Error	= LogLevel::Error,
				Silent	= LogLevel::Silent,
				Null
			);

			static OptionalLogLevel FromLogLevel(optional<LogLevel> level)
			{ return level ? (OptionalLogLevel::Enum)level->val() : OptionalLogLevel::Null; }

			static optional<LogLevel> ToLogLevel(OptionalLogLevel level)
			{
				if (level == Null)
					return null;
				return LogLevel((LogLevel::Enum)level.val());
			}

			STINGRAYKIT_DECLARE_ENUM_CLASS(OptionalLogLevel);
		};

	private:
		NamedLoggerParams				_params;
		mutable DuplicatingLogsFilter	_duplicatingLogsFilter;
		atomic<OptionalLogLevel>		_logLevel;
		Token							_token;

	public:
		NamedLogger(const char* name);
		~NamedLogger();

		const char* GetName() const { return _params.GetName(); }

		/// @brief Gets log level for NamedLogger
		/// @returns NamedLogger log level if it has one, or global Logger log level otherwise
		LogLevel GetLogLevel() const;

		/// @brief Sets or removes specific log level for NamedLogger
		/// @param logLevel log level value to set or null - to remove specific log level and use global one instead
		void SetLogLevel(optional<LogLevel> logLevel);

		bool BacktraceEnabled() const;
		void EnableBacktrace(bool enable);

		bool HighlightEnabled() const;
		void EnableHighlight(bool enable);

		LoggerStream Stream(LogLevel logLevel) const;

		LoggerStream Trace()	const { return Stream(LogLevel::Trace); }
		LoggerStream Debug()	const { return Stream(LogLevel::Debug); }
		LoggerStream Info()		const { return Stream(LogLevel::Info); }
		LoggerStream Warning()	const { return Stream(LogLevel::Warning); }
		LoggerStream Error()	const { return Stream(LogLevel::Error); }

		void Log(LogLevel logLevel, const std::string& text);
	};


	namespace Detail
	{
		struct NamedLoggerAccessor
		{
		private:
			NamedLogger* _logger;

		public:
			NamedLoggerAccessor() : _logger(null)
			{ }

			NamedLoggerAccessor(NamedLogger& logger) : _logger(&logger)
			{ }

			NamedLoggerAccessor& operator = (NullPtrType& n)
			{ return *this; }

			NamedLoggerAccessor& operator = (NamedLogger& logger)
			{ _logger = &logger; return *this; }

			LogLevel GetLogLevel() const			{ return _logger ? _logger->GetLogLevel() : Logger::GetLogLevel(); }

			LoggerStream Stream(LogLevel logLevel)	{ return _logger ? _logger->Stream(logLevel) : Logger::Stream(logLevel); }

			LoggerStream Trace()					{ return _logger ? _logger->Trace() : Logger::Trace(); }
			LoggerStream Debug()					{ return _logger ? _logger->Debug() : Logger::Debug(); }
			LoggerStream Info()						{ return _logger ? _logger->Info() : Logger::Info(); }
			LoggerStream Warning()					{ return _logger ? _logger->Warning() : Logger::Warning(); }
			LoggerStream Error()					{ return _logger ? _logger->Error() : Logger::Error(); }
		};
	}


	class ActionLogger
	{
	private:
		NamedLogger*	_namedLogger;
		LogLevel		_logLevel;
		std::string		_action;
		ElapsedTime		_elapsedTime;

	public:
		ActionLogger(const std::string& action);
		ActionLogger(LogLevel logLevel, const std::string& action);
		ActionLogger(NamedLogger& namedLogger, const std::string& action);
		ActionLogger(NamedLogger& namedLogger, LogLevel logLevel, const std::string& action);
		~ActionLogger();
	};


	namespace Detail {
	namespace LoggerDetail
	{
		extern NullPtrType	s_logger; // Static logger fallback
	}}


#define DETAIL_DECLARE_STATIC_LOGGER_ACCESSOR \
	::stingray::Detail::NamedLoggerAccessor STINGRAYKIT_CAT(detail_static_logger_accessor, __LINE__); \
	do { \
		using namespace ::stingray::Detail::LoggerDetail; \
		STINGRAYKIT_CAT(detail_static_logger_accessor, __LINE__) = s_logger; \
	} while (0)


#define STINGRAYKIT_STATIC_LOGGER \
	(STINGRAYKIT_CAT(detail_static_logger_accessor, __LINE__))

	class Tracer
	{
	private:
		Detail::NamedLoggerAccessor	_logger;
		LogLevel					_logLevel;
		const char*					_funcName;
		u64							_startTime;

	public:
		Tracer(const Detail::NamedLoggerAccessor& logger, const char* funcName);
		~Tracer();
	};


#if !defined(PRODUCTION_BUILD)
#	define TRACER	DETAIL_DECLARE_STATIC_LOGGER_ACCESSOR; stingray::Tracer tracer(STINGRAYKIT_STATIC_LOGGER, STINGRAYKIT_FUNCTION)
#else
#	define TRACER
#endif

	/** @} */

}

#endif
