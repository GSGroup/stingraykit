// Copyright (c) 2011 - 2022, GS Group, https://github.com/GSGroup
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

	namespace
	{

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
				bool operator () (const char* l, const char* r) const { return strcmp(l, r) < 0; }
			};

			using SettingsRegistry = std::map<std::string, NamedLoggerSettings>;
			using ObjectsRegistry = std::multimap<const char*, NamedLogger*, StrLess>;

		private:
			Mutex				_mutex;
			SettingsRegistry	_settings;
			ObjectsRegistry		_objects;

		public:
			ObjectsRegistry::const_iterator Register(NamedLogger* logger)
			{
				MutexLock l(_mutex);
				const SettingsRegistry::const_iterator it = _settings.find(logger->GetName());
				if (it != _settings.end())
				{
					logger->SetLogLevel(it->second.GetLogLevel());
					logger->EnableBacktrace(it->second.BacktraceEnabled());
					logger->EnableHighlight(it->second.HighlightEnabled());
				}
				return _objects.emplace(logger->GetName(), logger);
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

		class LoggerImpl
		{
			using SinksBundle = std::vector<ILoggerSinkPtr>;
			using LocalSinksBundle = inplace_vector<ILoggerSinkPtr, 16>;

		private:
			SinksBundle				_sinks;
			Mutex					_logMutex;
			NamedLoggerRegistry		_registry;

		public:
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

					LocalSinksBundle sinks;
					{
						MutexLock l(_logMutex);
						std::copy(_sinks.begin(), _sinks.end(), std::back_inserter(sinks));
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
			static void PutMessageToSinks(const LocalSinksBundle& sinks, const LoggerMessage& message)
			{
				for (const ILoggerSinkPtr& sink : sinks)
				{
					try
					{ sink->Log(message); }
					catch (const std::exception&)
					{ }
				}
			}
		};
		STINGRAYKIT_DECLARE_PTR(LoggerImpl);

		using LoggerSingleton = SafeSingleton<LoggerImpl>;

		u32 GlobalLogLevel = (u32)LogLevel::STINGRAY_DEFAULT_LOGLEVEL;

	}


	/////////////////////////////////////////////////////////////////


	void Logger::SetLogLevel(LogLevel logLevel)
	{
		AtomicU32::Store(GlobalLogLevel, (u32)logLevel.val(), MemoryOrderRelaxed);
		Stream(logLevel) << "Log level is " << logLevel;
	}


	LogLevel Logger::GetLogLevel()
	{ return (LogLevel::Enum)AtomicU32::Load(GlobalLogLevel, MemoryOrderRelaxed); }


	LoggerStream Logger::Stream(LogLevel logLevel, DuplicatingLogsFilter* duplicatingLogsFilter)
	{ return LoggerStream(null, GetLogLevel(), logLevel, duplicatingLogsFilter, &Logger::DoLog); }


	void Logger::SetLogLevel(const std::string& loggerName, optional<LogLevel> logLevel)
	{
		if (const LoggerImplPtr logger = LoggerSingleton::Instance())
			logger->GetRegistry().SetLogLevel(loggerName, logLevel);
	}


	void Logger::EnableBacktrace(const std::string& loggerName, bool enable)
	{
		if (const LoggerImplPtr logger = LoggerSingleton::Instance())
			logger->GetRegistry().EnableBacktrace(loggerName, enable);
	}


	void Logger::EnableHighlight(const std::string& loggerName, bool enable)
	{
		if (const LoggerImplPtr logger = LoggerSingleton::Instance())
			logger->GetRegistry().EnableHighlight(loggerName, enable);
	}


	std::set<std::string> Logger::GetLoggerNames()
	{
		if (const LoggerImplPtr logger = LoggerSingleton::Instance())
			return logger->GetRegistry().GetLoggerNames();
		else
			return std::set<std::string>();
	}


	Token Logger::AddSink(const ILoggerSinkPtr& sink)
	{
		const LoggerImplPtr logger = LoggerSingleton::Instance();
		if (!logger)
			return null;

		logger->AddSink(sink);
		return MakeFunctionToken(Bind(&LoggerImpl::RemoveSink, logger, sink));
	}


	void Logger::DoLog(const NamedLoggerParams* loggerParams, LogLevel logLevel, const std::string& text)
	{
		optional<LoggerMessage> msg;
		if (loggerParams)
			msg.emplace(loggerParams->GetName(), logLevel, loggerParams->BacktraceEnabled() ? StringBuilder() % text % ": " % Backtrace() : text, loggerParams->HighlightEnabled());
		else
			msg.emplace(logLevel, text);

		if (const LoggerImplPtr logger = LoggerSingleton::Instance())
			logger->Log(*msg);
		else
			SystemLogger::Log(*msg);
	}


	/////////////////////////////////////////////////////////////////


	NamedLogger::NamedLogger(const char* name, optional<LogLevel> logLevel)
		:	_params(name),
			_logLevel(OptionalLogLevel::FromLogLevel(logLevel))
	{
		if (const LoggerImplPtr logger = LoggerSingleton::Instance())
			_token = MakeFunctionToken(Bind(&NamedLoggerRegistry::Unregister, Bind(&LoggerImpl::GetRegistry, logger), logger->GetRegistry().Register(this)));
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

				try
				{
					const s64 elapsed = _elapsed.ElapsedMicroseconds();
					ms = elapsed / 1000;
					mms = elapsed % 1000;
				}
				catch (const std::exception&)
				{ }

				return StringFormat("%1%.%2$3%", ms, mms);
			}
		};

	}


	ActionLogger::ActionLogger(const std::string& action)
		: ActionLogger(LogLevel::Info, action)
	{ }


	ActionLogger::ActionLogger(LogLevel logLevel, const std::string& action)
		: _namedLogger(NULL), _logLevel(logLevel), _action(action)
	{ Logger::Stream(logLevel) << _action << "..."; }


	ActionLogger::ActionLogger(const NamedLogger& namedLogger, const std::string& action)
		: ActionLogger(namedLogger, LogLevel::Info, action)
	{ }


	ActionLogger::ActionLogger(const NamedLogger& namedLogger, LogLevel logLevel, const std::string& action)
		: _namedLogger(&namedLogger), _logLevel(logLevel), _action(action)
	{ _namedLogger->Stream(logLevel) << _action << "..."; }


	ActionLogger::~ActionLogger()
	{
		try
		{
			(_namedLogger ? _namedLogger->Stream(_logLevel) : Logger::Stream(_logLevel))
					<< _action << " completed" << (std::uncaught_exception() ? " with exception" : "") << " in " << ElapsedMillisecondsToString(_elapsedTime) << " ms";
		}
		catch (const std::exception&)
		{ }
	}


	/////////////////////////////////////////////////////////////////


	namespace Detail {
	namespace LoggerDetail
	{
		NullPtrType		s_logger; // Static logger fallback
	}}


	/////////////////////////////////////////////////////////////////


	Tracer::Tracer(const Detail::NamedLoggerAccessor& logger, const char* funcName)
		: _logger(logger), _funcName(funcName)
	{
		if (_logger.GetLogLevel() > LogLevel::Trace)
			return;

		_elapsedTime.emplace();
		_logger.Trace() << "TRACER: entering function '" << _funcName << "'";
	}


	Tracer::~Tracer()
	{
		if (!_elapsedTime)
			return;

		try
		{ _logger.Trace() << "TRACER: leaving function '" << _funcName << "'" << (std::uncaught_exception() ? " due to an exception" : "") << " (" << ElapsedMillisecondsToString(*_elapsedTime) << " ms)"; }
		catch (const std::exception&)
		{ }
	}

}
