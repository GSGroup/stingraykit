#ifndef STINGRAYKIT_LOG_LOGGERSTREAM_H
#define STINGRAYKIT_LOG_LOGGERSTREAM_H


#include <stingraykit/log/LogLevel.h>
#include <stingraykit/string/StringUtils.h>
#include <stingraykit/light_shared_ptr.h>
#include <stingraykit/string/string_stream.h>

namespace stingray
{

	class DuplicatingLogsFilter;

	namespace Detail
	{
		struct HideDuplicatingLogs
		{
			struct KeyType
			{
				const char* const	Filename;
				const int			Line;

				KeyType(const char* filename, int line) : Filename(filename), Line(line) { }

				bool operator < (const KeyType& other) const;
			};

			const int				Count;
			const KeyType			Key;
			DuplicatingLogsFilter	*Filter;

			HideDuplicatingLogs(int count, const char* filename, int line)
				: Count(count), Key(filename, line), Filter(NULL)
			{ }

			HideDuplicatingLogs(int count, const char* filename, int line, DuplicatingLogsFilter& filter)
				: Count(count), Key(filename, line), Filter(&filter)
			{ }
		};

	}

	class DuplicatingLogsFilter
	{
	public:
		struct StringCounter
		{
			std::string		Str;
			int				Count;

			StringCounter(const std::string& str = "") :Str(str), Count(-1) { }
			void Reset(const std::string& str) { Str = str; Count = 0; }
		};

	private:
		typedef Detail::HideDuplicatingLogs::KeyType		KeyType;
		typedef std::map<KeyType, StringCounter>			LastMessagesMap;

	private:
		LastMessagesMap		_lastMessages;

	public:
		StringCounter* Get(const KeyType& key) { return &_lastMessages[key]; }
	};


#define STINGRAYKIT_HIDE_DUPLICATING_LOGS(N_, ...) stingray::Detail::HideDuplicatingLogs(N_, __FILE__, __LINE__, ##__VA_ARGS__)


	class LoggerStream
	{
		STINGRAYKIT_NONASSIGNABLE(LoggerStream);
		typedef string_ostream							StreamType;

		typedef void LogFunction(const char* loggerName, LogLevel logLevel, const std::string& message);

		const char*										_loggerName;
		LogLevel										_loggerLogLevel;
		LogLevel										_streamLogLevel;
		light_shared_ptr<StreamType>					_stream;
		DuplicatingLogsFilter*							_duplicatingLogsFilter;
		light_shared_ptr<Detail::HideDuplicatingLogs>	_hideDuplicatingLogs;
		LogFunction*									_logFunction;

	public:
		LoggerStream(const char* loggerName, LogLevel loggerLogLevel, LogLevel streamLogLevel, DuplicatingLogsFilter* duplicatingLogsFilter, LogFunction* logFunction);
		~LoggerStream();

		template < typename T >
		typename EnableIf<!IsIntType<T>::Value, LoggerStream&>::ValueT operator << (const T& val)
		{
			if (_streamLogLevel < _loggerLogLevel)
				return *this;

			if (!_stream)
				_stream.reset(new StreamType);
			ToString(*_stream, val);
			return *this;
		}

		template < typename T >
		typename EnableIf<IsIntType<T>::Value, LoggerStream&>::ValueT operator << (T val)
		{
			if (_streamLogLevel < _loggerLogLevel)
				return *this;

			if (!_stream)
				_stream.reset(new StreamType);
			ToString(*_stream, val);
			return *this;
		}

		LoggerStream& operator << (const Detail::HideDuplicatingLogs& val)
		{
			if (val.Filter)
				_duplicatingLogsFilter = val.Filter;

			if (_duplicatingLogsFilter)
				_hideDuplicatingLogs.reset(new Detail::HideDuplicatingLogs(val));

			return *this;
		}

	private:
		void DoLog(const std::string& message);
		void DoLogImpl(const std::string& message);
	};

}

#endif
