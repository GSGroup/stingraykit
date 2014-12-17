#include <stingray/toolkit/log/Logger.h>

#include <cstdio>
#include <cstring>

#include <map>
#include <sstream>

#include <stingray/toolkit/log/SystemLogger.h>
#include <stingray/toolkit/Backtrace.h>
#include <stingray/toolkit/FunctionToken.h>
#include <stingray/toolkit/PhoenixSingleton.h>
#include <stingray/toolkit/SafeSingleton.h>
#include <stingray/toolkit/StringFormat.h>
#include <stingray/toolkit/collection/iterators.h>

namespace stingray
{

	namespace Detail {
	namespace LoggerDetail
	{
		NullPtrType		s_logger; // Static logger fallback
	}}


	class NamedLoggerRegistry : public PhoenixSingleton<NamedLoggerRegistry>
	{
		TOOLKIT_PHOENIXSINGLETON(NamedLoggerRegistry);

		struct StrLess
		{
			bool operator() (const char* l, const char* r) const { return strcmp(l, r) < 0; }
		};

		typedef std::multimap<const char*, NamedLogger*, StrLess> LoggerRegistry;

	private:
		LoggerRegistry	_registry;
		Mutex			_registryMutex;

		NamedLoggerRegistry() { }

	public:
		void Register(const char* loggerName, NamedLogger* logger)
		{
			MutexLock l(_registryMutex);
			_registry.insert(std::make_pair(loggerName, logger));
		}

		void Unregister(const char* loggerName, NamedLogger* logger)
		{
			MutexLock l(_registryMutex);
			std::pair<LoggerRegistry::iterator, LoggerRegistry::iterator> range = _registry.equal_range(loggerName);
			for (LoggerRegistry::iterator it = range.first; it != range.second; ++it)
			{
				if (it->second == logger)
				{
					_registry.erase(it);
					return;
				}
			}
			SystemLogger::Log(LogLevel::Error, StringBuilder() % "Unknown named logger: " % loggerName);
		}

		void GetLoggerNames(std::set<std::string>& out) const
		{
			MutexLock l(_registryMutex);
			out.clear();
			std::copy(keys_iterator(_registry.begin()), keys_iterator(_registry.end()), std::inserter(out, out.begin()));
		}

		void SetLogLevel(const std::string& loggerName, LogLevel logLevel)
		{
			MutexLock l(_registryMutex);
			std::pair<LoggerRegistry::iterator, LoggerRegistry::iterator> range = _registry.equal_range(loggerName.c_str());
			for (LoggerRegistry::iterator it = range.first; it != range.second; ++it)
				it->second->SetLogLevel(logLevel);
		}

		LogLevel GetLogLevel(const std::string& loggerName)
		{
			MutexLock l(_registryMutex);
			LoggerRegistry::const_iterator it = _registry.find(loggerName.c_str());
			if (it == _registry.end())
				return Logger::GetLogLevel();
			return it->second->GetLogLevel();
		}

		void SetBacktraceEnabled(const std::string& loggerName, bool enable)
		{
			MutexLock l(_registryMutex);
			std::pair<LoggerRegistry::iterator, LoggerRegistry::iterator> range = _registry.equal_range(loggerName.c_str());
			for (LoggerRegistry::iterator it = range.first; it != range.second; ++it)
				it->second->EnableBacktrace(enable);
		}

		bool GetBacktraceEnabled(const std::string& loggerName)
		{
			MutexLock l(_registryMutex);
			LoggerRegistry::const_iterator it = _registry.find(loggerName.c_str());
			if (it == _registry.end())
				return false;
			return it->second->BacktraceEnabled();
		}
	};


	class LoggerImpl
	{
		typedef std::vector<ILoggerSinkPtr> SinksBundle;

	private:
		SinksBundle					_sinks;
		atomic<LogLevel>			_logLevel;
		Mutex						_logMutex;

	public:
		LoggerImpl() : _logLevel(LogLevel::Info)
		{ }


		~LoggerImpl()
		{ }


		void AddSink(const ILoggerSinkPtr& sink)
		{
			MutexLock l(_logMutex);
			_sinks.push_back(sink);
		}


		void RemoveSink(const ILoggerSinkPtr& sink)
		{
			MutexLock l(_logMutex);
			SinksBundle::iterator it = std::find(_sinks.begin(), _sinks.end(), sink);
			if (it != _sinks.end())
				_sinks.erase(it);
		}


		LogLevel GetLogLevel() const throw()
		{ return _logLevel.load(memory_order_relaxed); }


		void SetLogLevel(LogLevel logLevel) throw()
		{ _logLevel.store(logLevel, memory_order_relaxed); }


		void Log(LogLevel logLevel, const std::string& message) throw()
		{
			try
			{
				EnableInterruptionPoints eip(false);
				DoLog(logLevel, message);
			}
			catch (const std::exception&)
			{ }
		}


		void Log(const std::string& loggerName, LogLevel logLevel, const std::string& message) throw()
		{
			try
			{
				EnableInterruptionPoints eip(false);
				DoLog(loggerName, logLevel, message);
			}
			catch (const std::exception&)
			{ }
		}

	private:
		void DoLog(LogLevel logLevel, const std::string& message)
		{
			SinksBundle sinks;

			{
				MutexLock l(_logMutex);
				sinks = _sinks;
			}

			if (sinks.empty())
				SystemLogger::Log(logLevel, message);
			else
				DoLog(sinks, LoggerMessage(logLevel, message));
		}


		void DoLog(const std::string& loggerName, LogLevel logLevel, const std::string& message)
		{
			SinksBundle sinks;

			{
				MutexLock l(_logMutex);
				sinks = _sinks;
			}

			if (sinks.empty())
				SystemLogger::Log(loggerName, logLevel, message);
			else
				DoLog(sinks, LoggerMessage(loggerName, logLevel, message));
		}


		void DoLog(const SinksBundle& sinks, const LoggerMessage& message)
		{
			for (SinksBundle::const_iterator it = sinks.begin(); it != sinks.end(); ++it)
			{
				try
				{
					ILoggerSinkPtr sink = *it;
					sink->Log(message);
				}
				catch (const std::exception&)
				{ }
			}
		}
	};
	TOOLKIT_DECLARE_PTR(LoggerImpl);


	typedef SafeSingleton<LoggerImpl> LoggerSingleton;


	/////////////////////////////////////////////////////////////////


	NamedLogger::NamedLogger(const char* name) :
		_name(name),
		_logLevel(OptionalLogLevel::Null),
		_backtrace(false)
	{ NamedLoggerRegistry::Instance().Register(_name, this); }

	NamedLogger::~NamedLogger()
	{ NamedLoggerRegistry::Instance().Unregister(_name, this); }


	/////////////////////////////////////////////////////////////////


	struct LogLevelHolder
	{
		static u32 s_logLevel;
	};


	u32 LogLevelHolder::s_logLevel = (u32)LogLevel::Info;


	void Logger::SetLogLevel(LogLevel logLevel)
	{ Atomic::Store(LogLevelHolder::s_logLevel, (u32)logLevel.val(), memory_order_relaxed); }


	LogLevel Logger::GetLogLevel()
	{ return (LogLevel::Enum)Atomic::Load(LogLevelHolder::s_logLevel, memory_order_relaxed); }


	struct LoggerStreamHelper
	{
		static void Log(const char* loggerName, LogLevel logLevel, const std::string& message)
		{
			if (loggerName)
				Logger::Log(loggerName, logLevel, message);
			else
				Logger::Log(logLevel, message);
		}
	};


	LoggerStream Logger::Stream(LogLevel logLevel, DuplicatingLogsFilter* duplicatingLogsFilter)
	{ return LoggerStream(null, GetLogLevel(), logLevel, duplicatingLogsFilter, &LoggerStreamHelper::Log); }


	void Logger::SetLogLevel(const std::string& loggerName, LogLevel logLevel)
	{ NamedLoggerRegistry::Instance().SetLogLevel(loggerName, logLevel); }


	LogLevel Logger::GetLogLevel(const std::string& loggerName)
	{ return NamedLoggerRegistry::Instance().GetLogLevel(loggerName); }


	void Logger::SetBacktraceEnabled(const std::string& loggerName, bool enable)
	{ NamedLoggerRegistry::Instance().SetBacktraceEnabled(loggerName, enable); }


	bool Logger::GetBacktraceEnabled(const std::string& loggerName)
	{ return NamedLoggerRegistry::Instance().GetBacktraceEnabled(loggerName); }


	void Logger::GetLoggerNames(std::set<std::string>& out)
	{ NamedLoggerRegistry::Instance().GetLoggerNames(out); }


	ITokenPtr Logger::AddSink(const ILoggerSinkPtr& sink)
	{
		LoggerImplPtr logger = LoggerSingleton::Instance();
		if (!logger)
			return null;

		logger->AddSink(sink);
		return make_shared<FunctionToken>(bind(&LoggerImpl::RemoveSink, logger, sink));
	}


	void Logger::Log(LogLevel logLevel, const std::string& message)
	{
		LoggerImplPtr logger = LoggerSingleton::Instance();
		LogLevel ll = logger ? logger->GetLogLevel() : LogLevel(LogLevel::Debug);
		if (logLevel < ll)
			return;

		if (logger)
			return logger->Log(logLevel, message);
		else
			SystemLogger::Log(logLevel, message);
	}


	void Logger::Log(const std::string& loggerName, LogLevel logLevel, const std::string& message)
	{
		LoggerImplPtr logger = LoggerSingleton::Instance();
		if (logger)
			logger->Log(loggerName, logLevel, message + (GetBacktraceEnabled(loggerName) ? ": " + Backtrace().Get() : std::string()));
		else
			SystemLogger::Log(loggerName, logLevel, message + (GetBacktraceEnabled(loggerName) ? ": " + Backtrace().Get() : std::string()));
	}


	LoggerStream NamedLogger::Stream(LogLevel logLevel)
	{ return LoggerStream(_name, GetLogLevel(), logLevel, &_duplicatingLogsFilter, &LoggerStreamHelper::Log); }


	/////////////////////////////////////////////////////////////////


	ActionLogger::ActionLogger(const std::string& action)
		: _namedLogger(NULL), _action(action)
	{ Logger::Info() << _action << "..."; }

	ActionLogger::ActionLogger(NamedLogger& namedLogger, const std::string& action)
		: _namedLogger(&namedLogger), _action(action)
	{ _namedLogger->Info() << _action << "..."; }

	ActionLogger::~ActionLogger()
	{
		s64 ms = 0;
		int mms = 0;
		try { s64 e = _elapsedTime.ElapsedMicroseconds(); ms = e / 1000; mms = e % 1000; } catch(const std::exception&) { }
		if (std::uncaught_exception())
		{
			try
			{
				if (_namedLogger)
					_namedLogger->Info() << _action << " completed with exception in " << ms << "." << mms << " ms";
				else
					Logger::Info() << _action << " completed with exception in " << ms << "." << mms << " ms";
			}
			catch (const std::exception&)
			{ return; }
		}
		else
		{
			if (_namedLogger)
				_namedLogger->Info() << _action << " completed in " << ms << "." << mms << " ms";
			else
				Logger::Info() << _action << " completed in " << ms << "." << mms << " ms";
		}
	}


	/////////////////////////////////////////////////////////////////


	Tracer::Tracer(const Detail::NamedLoggerAccessor& logger, const char* funcName)
		: _logger(logger), _funcName(funcName)
	{
		if (_logger.GetLogLevel() >= LogLevel::Trace)
			_logger.Trace() << "TRACER: entering function '" <<  _funcName << "'";
	}


	Tracer::~Tracer()
	{
		s64 ms = 0;
		int mms = 0;
		try { s64 e = _elapsedTime.ElapsedMicroseconds(); ms = e / 1000; mms = e % 1000; } catch(const std::exception&) {}
		if (std::uncaught_exception())
		{
			try
			{
				if (_logger.GetLogLevel() >= LogLevel::Trace)
					_logger.Trace() << "TRACER: leaving function '" << _funcName << "' due to an exception (" << StringFormat("%1%.%2$3%", ms, mms) << " ms)";
			}
			catch(const std::exception&)
			{ return; }
		}
		else
		{
			if (_logger.GetLogLevel() >= LogLevel::Trace)
				_logger.Trace() << "TRACER: leaving function '" << _funcName << "' (" << StringFormat("%1%.%2$3%", ms, mms) << " ms)";
		}

	}


	/////////////////////////////////////////////////////////////////


	void LogException(const std::exception& ex)
	{ Logger::Error() << diagnostic_information(ex); }


	void LogExceptionTo(NamedLogger& namedLogger, const std::exception& ex)
	{ namedLogger.Error() << diagnostic_information(ex); }


	/////////////////////////////////////////////////////////////////


	namespace Detail {
	namespace LoggerDetail
	{
		LogExceptionsGuard::~LogExceptionsGuard()
		{
			if (!std::uncaught_exception())
				return;

			try
			{ Logger::Error() << "An exception occured while calling '" << _text << "'!"; }
			catch(const std::exception&)
			{ return; }
		}
	}}
}
