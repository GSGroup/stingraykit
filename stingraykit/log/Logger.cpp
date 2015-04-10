// Copyright (c) 2011 - 2015, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/log/Logger.h>

#include <cstdio>
#include <cstring>

#include <map>
#include <sstream>

#include <stingraykit/log/SystemLogger.h>
#include <stingraykit/diagnostics/Backtrace.h>
#include <stingraykit/FunctionToken.h>
#include <stingraykit/PhoenixSingleton.h>
#include <stingraykit/SafeSingleton.h>
#include <stingraykit/string/StringFormat.h>
#include <stingraykit/collection/iterators.h>

namespace stingray
{

	namespace Detail {
	namespace LoggerDetail
	{
		NullPtrType		s_logger; // Static logger fallback
	}}


	struct NamedLoggerSettings
	{
	private:
		optional<LogLevel>	_logLevel;
		bool				_backtrace;
		bool				_highlight;

	public:
		NamedLoggerSettings() :
			_backtrace(false), _highlight(false)
		{ }

		optional<LogLevel> GetLogLevel() const			{ return _logLevel; }
		void SetLogLevel(optional<LogLevel> logLevel)	{ _logLevel = logLevel; }

		bool BacktraceEnabled() const					{ return _backtrace; }
		void EnableBacktrace(bool enable)				{ _backtrace = enable; }

		bool HighlightEnabled() const					{ return _highlight; }
		void EnableHighlight(bool enable)				{ _highlight = enable; }

		bool IsEmpty() const							{ return GetLogLevel() || BacktraceEnabled() || HighlightEnabled(); }
	};


	class NamedLoggerRegistry
	{
		struct StrLess
		{
			bool operator() (const char* l, const char* r) const { return strcmp(l, r) < 0; }
		};

		typedef std::map<std::string, NamedLoggerSettings>			SettingsRegistry;
		typedef std::multimap<const char*, NamedLogger*, StrLess>	ObjectsRegistry;

	private:
		Mutex				_mutex;
		SettingsRegistry	_settings;
		ObjectsRegistry		_objects;

	public:
		NamedLoggerRegistry()
		{ }

		ObjectsRegistry::iterator Register(const char* loggerName, NamedLogger* logger)
		{
			MutexLock l(_mutex);
			SettingsRegistry::iterator it = _settings.find(loggerName);
			if (it != _settings.end())
			{
				logger->SetLogLevel(it->second.GetLogLevel());
				logger->EnableBacktrace(it->second.BacktraceEnabled());
				logger->EnableHighlight(it->second.HighlightEnabled());
			}
			return _objects.insert(std::make_pair(loggerName, logger));
		}

		void Unregister(ObjectsRegistry::iterator it)
		{
			MutexLock l(_mutex);
			_objects.erase(it);
		}

		void GetLoggerNames(std::set<std::string>& out) const
		{
			MutexLock l(_mutex);
			out.clear();
			std::copy(keys_iterator(_objects.begin()), keys_iterator(_objects.end()), std::inserter(out, out.begin()));
		}

		void SetLogLevel(const std::string& loggerName, optional<LogLevel> logLevel)
		{
			MutexLock l(_mutex);
			std::pair<ObjectsRegistry::iterator, ObjectsRegistry::iterator> range = _objects.equal_range(loggerName.c_str());
			for (ObjectsRegistry::iterator it = range.first; it != range.second; ++it)
				it->second->SetLogLevel(logLevel);

			SettingsRegistry::iterator it = _settings.find(loggerName);
			if (it == _settings.end())
				it = _settings.insert(std::make_pair(loggerName, NamedLoggerSettings())).first;
			it->second.SetLogLevel(logLevel);
			if (it->second.IsEmpty())
				_settings.erase(it);
		}

		void EnableBacktrace(const std::string& loggerName, bool enable)
		{
			MutexLock l(_mutex);
			std::pair<ObjectsRegistry::iterator, ObjectsRegistry::iterator> range = _objects.equal_range(loggerName.c_str());
			for (ObjectsRegistry::iterator it = range.first; it != range.second; ++it)
				it->second->EnableBacktrace(enable);

			SettingsRegistry::iterator it = _settings.find(loggerName);
			if (it == _settings.end())
				it = _settings.insert(std::make_pair(loggerName, NamedLoggerSettings())).first;
			it->second.EnableBacktrace(enable);
			if (it->second.IsEmpty())
				_settings.erase(it);
		}

		void EnableHighlight(const std::string& loggerName, bool enable)
		{
			MutexLock l(_mutex);
			std::pair<ObjectsRegistry::iterator, ObjectsRegistry::iterator> range = _objects.equal_range(loggerName.c_str());
			for (ObjectsRegistry::iterator it = range.first; it != range.second; ++it)
				it->second->EnableHighlight(enable);

			SettingsRegistry::iterator it = _settings.find(loggerName);
			if (it == _settings.end())
				it = _settings.insert(std::make_pair(loggerName, NamedLoggerSettings())).first;
			it->second.EnableHighlight(enable);
			if (it->second.IsEmpty())
				_settings.erase(it);
		}
	};
	STINGRAYKIT_DECLARE_PTR(NamedLoggerRegistry);


	class LoggerImpl
	{
		typedef std::vector<ILoggerSinkPtr>							SinksBundle;

	private:
		SinksBundle				_sinks;
		Mutex					_logMutex;
		NamedLoggerRegistry		_registry;

	public:
		LoggerImpl()
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


		void Log(const LoggerMessage& message) throw()
		{
			try
			{
				EnableInterruptionPoints eip(false);

				SinksBundle sinks;
				{
					MutexLock l(_logMutex);
					sinks = _sinks;
				}

				if (sinks.empty())
					SystemLogger::Log(message);
				else
					PutMessageToSinks(sinks, message);
			}
			catch (const std::exception&)
			{ }
		}


		NamedLoggerRegistry& GetRegistry()
		{ return _registry; }


	private:
		static void PutMessageToSinks(const SinksBundle& sinks, const LoggerMessage& message)
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
	STINGRAYKIT_DECLARE_PTR(LoggerImpl);


	typedef SafeSingleton<LoggerImpl> LoggerSingleton;


	/////////////////////////////////////////////////////////////////


	NamedLogger::NamedLogger(const char* name) :
		_params(name),
		_logLevel(OptionalLogLevel::Null)
	{
		LoggerImplPtr logger = LoggerSingleton::Instance();
		if (logger)
		{
			NamedLoggerRegistryPtr r(logger, &(logger->GetRegistry()));
			_token = MakeToken<FunctionToken>(bind(&NamedLoggerRegistry::Unregister, r, r->Register(_params.GetName(), this)));
		}
	}


	NamedLogger::~NamedLogger()
	{ _token.Reset(); }


	/////////////////////////////////////////////////////////////////


	struct LogLevelHolder
	{
		static u32 s_logLevel;
	};


	u32 LogLevelHolder::s_logLevel = (u32)LogLevel::Info;


	void Logger::SetLogLevel(LogLevel logLevel)
	{
		Atomic::Store(LogLevelHolder::s_logLevel, (u32)logLevel.val(), memory_order_relaxed);
		Stream(logLevel) << "Log level is " << logLevel;
	}


	LogLevel Logger::GetLogLevel()
	{ return (LogLevel::Enum)Atomic::Load(LogLevelHolder::s_logLevel, memory_order_relaxed); }


	LoggerStream Logger::Stream(LogLevel logLevel, DuplicatingLogsFilter* duplicatingLogsFilter)
	{ return LoggerStream(null, GetLogLevel(), logLevel, duplicatingLogsFilter, &Logger::DoLog); }


	void Logger::SetLogLevel(const std::string& loggerName, optional<LogLevel> logLevel)
	{
		LoggerImplPtr logger = LoggerSingleton::Instance();
		if (logger)
			logger->GetRegistry().SetLogLevel(loggerName, logLevel);
	}


	void Logger::EnableBacktrace(const std::string& loggerName, bool enable)
	{
		LoggerImplPtr logger = LoggerSingleton::Instance();
		if (logger)
			logger->GetRegistry().EnableBacktrace(loggerName, enable);
	}


	void Logger::EnableHighlight(const std::string& loggerName, bool enable)
	{
		LoggerImplPtr logger = LoggerSingleton::Instance();
		if (logger)
			logger->GetRegistry().EnableHighlight(loggerName, enable);
	}


	void Logger::GetLoggerNames(std::set<std::string>& out)
	{
		LoggerImplPtr logger = LoggerSingleton::Instance();
		if (logger)
			logger->GetRegistry().GetLoggerNames(out);
	}


	Token Logger::AddSink(const ILoggerSinkPtr& sink)
	{
		LoggerImplPtr logger = LoggerSingleton::Instance();
		if (!logger)
			return null;

		logger->AddSink(sink);
		return MakeToken<FunctionToken>(bind(&LoggerImpl::RemoveSink, logger, sink));
	}


	void Logger::DoLog(const NamedLoggerParams* loggerParams, LogLevel logLevel, const std::string& text)
	{
		LoggerImplPtr logger = LoggerSingleton::Instance();
		LogLevel ll = logger ? GetLogLevel() : LogLevel(LogLevel::Debug);
		if (!loggerParams && logLevel < ll) // NamedLogger LoggerStream checks the log level in its destructor
			return;

		optional<LoggerMessage> msg;
		if (loggerParams)
			msg = LoggerMessage(loggerParams->GetName(), logLevel, text + (loggerParams->BacktraceEnabled() ? ": " + Backtrace().Get() : std::string()), loggerParams->HighlightEnabled());
		else
			msg = LoggerMessage(logLevel, text, false);

		if (logger)
			logger->Log(*msg);
		else
			SystemLogger::Log(*msg);
	}


	LogLevel NamedLogger::GetLogLevel() const
	{
		optional<LogLevel> logLevel = OptionalLogLevel::ToLogLevel(_logLevel.load(memory_order_relaxed));
		return logLevel ? *logLevel : Logger::GetLogLevel();
	}


	void NamedLogger::SetLogLevel(optional<LogLevel> logLevel)
	{
		_logLevel.store(OptionalLogLevel::FromLogLevel(logLevel), memory_order_relaxed);
		Stream(GetLogLevel()) << "Log level is " << logLevel;
	}


	bool NamedLogger::BacktraceEnabled() const
	{ return _params.BacktraceEnabled(); }


	void NamedLogger::EnableBacktrace(bool enable)
	{
		_params.EnableBacktrace(enable);
		Stream(GetLogLevel()) << "Backtrace " << (enable ? "enabled" : "disabled");
	}


	bool NamedLogger::HighlightEnabled() const
	{ return _params.HighlightEnabled(); }


	void NamedLogger::EnableHighlight(bool enable)
	{ _params.EnableHighlight(enable); }


	LoggerStream NamedLogger::Stream(LogLevel logLevel) const
	{ return LoggerStream(&_params, GetLogLevel(), logLevel, &_duplicatingLogsFilter, &Logger::DoLog); }


	void NamedLogger::Log(LogLevel logLevel, const std::string& message)
	{ Stream(logLevel) << message; }

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

}
