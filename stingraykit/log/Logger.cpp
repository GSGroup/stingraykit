// Copyright (c) 2011 - 2019, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/log/Logger.h>

#include <stingraykit/log/SystemLogger.h>
#include <stingraykit/string/StringFormat.h>
#include <stingraykit/time/TimeEngine.h>
#include <stingraykit/FunctionToken.h>
#include <stingraykit/SafeSingleton.h>
#include <stingraykit/lazy.h>

#include <string.h>

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
		NamedLoggerSettings()
			: _backtrace(false), _highlight(false)
		{ }

		optional<LogLevel> GetLogLevel() const			{ return _logLevel; }
		void SetLogLevel(optional<LogLevel> logLevel)	{ _logLevel = logLevel; }

		bool BacktraceEnabled() const					{ return _backtrace; }
		void EnableBacktrace(bool enable)				{ _backtrace = enable; }

		bool HighlightEnabled() const					{ return _highlight; }
		void EnableHighlight(bool enable)				{ _highlight = enable; }

		bool IsEmpty() const							{ return !_logLevel && !_backtrace && !_highlight; }
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

		ObjectsRegistry::const_iterator Register(const char* loggerName, NamedLogger* logger)
		{
			MutexLock l(_mutex);
			const SettingsRegistry::const_iterator it = _settings.find(loggerName);
			if (it != _settings.end())
			{
				logger->SetLogLevel(it->second.GetLogLevel());
				logger->EnableBacktrace(it->second.BacktraceEnabled());
				logger->EnableHighlight(it->second.HighlightEnabled());
			}
			return _objects.emplace(loggerName, logger);
		}

		void Unregister(ObjectsRegistry::const_iterator it)
		{
			MutexLock l(_mutex);
			_objects.erase(it);
		}

		std::set<std::string> GetLoggerNames() const
		{
			MutexLock l(_mutex);
			return std::set<std::string>(keys_iterator(_objects.begin()), keys_iterator(_objects.end()));
		}

		void SetLogLevel(const std::string& loggerName, optional<LogLevel> logLevel)
		{
			MutexLock l(_mutex);
			const std::pair<ObjectsRegistry::iterator, ObjectsRegistry::iterator> range = _objects.equal_range(loggerName.c_str());
			for (ObjectsRegistry::iterator it = range.first; it != range.second; ++it)
				it->second->SetLogLevel(logLevel);

			const SettingsRegistry::iterator it = _settings.emplace(loggerName, NamedLoggerSettings()).first;
			it->second.SetLogLevel(logLevel);
			if (it->second.IsEmpty())
				_settings.erase(it);
		}

		void EnableBacktrace(const std::string& loggerName, bool enable)
		{
			MutexLock l(_mutex);
			const std::pair<ObjectsRegistry::iterator, ObjectsRegistry::iterator> range = _objects.equal_range(loggerName.c_str());
			for (ObjectsRegistry::iterator it = range.first; it != range.second; ++it)
				it->second->EnableBacktrace(enable);

			const SettingsRegistry::iterator it = _settings.emplace(loggerName, NamedLoggerSettings()).first;
			it->second.EnableBacktrace(enable);
			if (it->second.IsEmpty())
				_settings.erase(it);
		}

		void EnableHighlight(const std::string& loggerName, bool enable)
		{
			MutexLock l(_mutex);
			const std::pair<ObjectsRegistry::iterator, ObjectsRegistry::iterator> range = _objects.equal_range(loggerName.c_str());
			for (ObjectsRegistry::iterator it = range.first; it != range.second; ++it)
				it->second->EnableHighlight(enable);

			const SettingsRegistry::iterator it = _settings.emplace(loggerName, NamedLoggerSettings()).first;
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

		void Log(const LoggerMessage& message) noexcept
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
				{ (*it)->Log(message); }
				catch (const std::exception&)
				{ }
			}
		}
	};
	STINGRAYKIT_DECLARE_PTR(LoggerImpl);


	typedef SafeSingleton<LoggerImpl> LoggerSingleton;


	/////////////////////////////////////////////////////////////////


	struct LogLevelHolder
	{
		static u32 s_logLevel;
	};


	u32 LogLevelHolder::s_logLevel = (u32)LogLevel::Info;


	void Logger::SetLogLevel(LogLevel logLevel)
	{
		AtomicU32::Store(LogLevelHolder::s_logLevel, (u32)logLevel.val(), MemoryOrderRelaxed);
		Stream(logLevel) << "Log level is " << logLevel;
	}


	LogLevel Logger::GetLogLevel()
	{ return (LogLevel::Enum)AtomicU32::Load(LogLevelHolder::s_logLevel, MemoryOrderRelaxed); }


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


	std::set<std::string> Logger::GetLoggerNames()
	{
		LoggerImplPtr logger = LoggerSingleton::Instance();
		if (logger)
			return logger->GetRegistry().GetLoggerNames();
		else
			return std::set<std::string>();
	}


	Token Logger::AddSink(const ILoggerSinkPtr& sink)
	{
		LoggerImplPtr logger = LoggerSingleton::Instance();
		if (!logger)
			return null;

		logger->AddSink(sink);
		return MakeFunctionToken(Bind(&LoggerImpl::RemoveSink, logger, sink));
	}


	void Logger::DoLog(const NamedLoggerParams* loggerParams, LogLevel logLevel, const std::string& text)
	{
		const LoggerImplPtr logger = LoggerSingleton::Instance();
		const LogLevel ll = logger ? GetLogLevel() : LogLevel(LogLevel::Info);
		if (!loggerParams && logLevel < ll) // NamedLogger LoggerStream checks the log level in its destructor
			return;

		optional<LoggerMessage> msg;
		if (loggerParams)
			msg.emplace(loggerParams->GetName(), logLevel, loggerParams->BacktraceEnabled() ? StringBuilder() % text % ": " % Backtrace() : text, loggerParams->HighlightEnabled());
		else
			msg.emplace(logLevel, text, false);

		if (logger)
			logger->Log(*msg);
		else
			SystemLogger::Log(*msg);
	}


	/////////////////////////////////////////////////////////////////


	NamedLogger::NamedLogger(const char* name)
		:	_params(name),
			_logLevel(OptionalLogLevel::Null)
	{
		if (const LoggerImplPtr logger = LoggerSingleton::Instance())
		{
			const NamedLoggerRegistryPtr r(logger, &(logger->GetRegistry()));
			_token = MakeFunctionToken(Bind(&NamedLoggerRegistry::Unregister, r, r->Register(_params.GetName(), this)));
		}
	}


	LogLevel NamedLogger::GetLogLevel() const
	{ return OptionalLogLevel::ToLogLevel(_logLevel.load(MemoryOrderRelaxed)).get_value_or(lazy(&Logger::GetLogLevel)); }


	void NamedLogger::SetLogLevel(optional<LogLevel> logLevel)
	{
		_logLevel.store(OptionalLogLevel::FromLogLevel(logLevel), MemoryOrderRelaxed);
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


	void NamedLogger::Log(LogLevel logLevel, const std::string& message) const
	{ Stream(logLevel) << message; }

	/////////////////////////////////////////////////////////////////

	namespace
	{

		class ElapsedMillisecondsToString
		{
		private:
			const ElapsedTime&	_elapsed;

		public:
			explicit ElapsedMillisecondsToString(const ElapsedTime& elapsed) : _elapsed(elapsed) { }

			std::string ToString() const
			{
				s64 ms = 0;
				int mms = 0;

				try { s64 e = _elapsed.ElapsedMicroseconds(); ms = e / 1000; mms = e % 1000; }
				catch (const std::exception&) { }

				return StringBuilder() % ms % "." % mms;
			}
		};

	}


	ActionLogger::ActionLogger(const std::string& action)
		: _namedLogger(NULL), _logLevel(LogLevel::Info), _action(action)
	{ Logger::Info() << _action << "..."; }


	ActionLogger::ActionLogger(LogLevel logLevel, const std::string& action)
		: _namedLogger(NULL), _logLevel(logLevel), _action(action)
	{ Logger::Stream(logLevel) << _action << "..."; }


	ActionLogger::ActionLogger(const NamedLogger& namedLogger, const std::string& action)
		: _namedLogger(&namedLogger), _logLevel(LogLevel::Info), _action(action)
	{ _namedLogger->Info() << _action << "..."; }


	ActionLogger::ActionLogger(const NamedLogger& namedLogger, LogLevel logLevel, const std::string& action)
		: _namedLogger(&namedLogger), _logLevel(logLevel), _action(action)
	{ _namedLogger->Stream(logLevel) << _action << "..."; }


	ActionLogger::~ActionLogger()
	{
		if (std::uncaught_exception())
		{
			try
			{
				if (_namedLogger)
					_namedLogger->Stream(_logLevel) << _action << " completed with exception in " << ElapsedMillisecondsToString(_elapsedTime) << " ms";
				else
					Logger::Stream(_logLevel) << _action << " completed with exception in " << ElapsedMillisecondsToString(_elapsedTime) << " ms";
			}
			catch (const std::exception&)
			{ return; }
		}
		else
		{
			if (_namedLogger)
				_namedLogger->Stream(_logLevel) << _action << " completed in " << ElapsedMillisecondsToString(_elapsedTime) << " ms";
			else
				Logger::Stream(_logLevel) << _action << " completed in " << ElapsedMillisecondsToString(_elapsedTime) << " ms";
		}
	}


	/////////////////////////////////////////////////////////////////


	Tracer::Tracer(const Detail::NamedLoggerAccessor& logger, const char* funcName)
		: _logger(logger), _logLevel(logger.GetLogLevel()), _funcName(funcName)
	{
		if (_logLevel > LogLevel::Trace)
			return;

		_startTime = TimeEngine::GetMonotonicMicroseconds();
		_logger.Trace() << "TRACER: entering function '" << _funcName << "'";
	}


	Tracer::~Tracer()
	{
		if (_logLevel > LogLevel::Trace)
			return;

		try
		{
			s64 e = TimeEngine::GetMonotonicMicroseconds() - _startTime;
			s64 ms = e / 1000;
			int mms = e % 1000;

			if (std::uncaught_exception())
				_logger.Trace() << "TRACER: leaving function '" << _funcName << "' due to an exception (" << StringFormat("%1%.%2$3%", ms, mms) << " ms)";
			else
				_logger.Trace() << "TRACER: leaving function '" << _funcName << "' (" << StringFormat("%1%.%2$3%", ms, mms) << " ms)";
		}
		catch (const std::exception&)
		{ }
	}

}
