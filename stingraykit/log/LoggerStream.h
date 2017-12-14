#ifndef STINGRAYKIT_LOG_LOGGERSTREAM_H
#define STINGRAYKIT_LOG_LOGGERSTREAM_H

// Copyright (c) 2011 - 2017, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.


#include <stingraykit/log/LogLevel.h>
#include <stingraykit/log/NamedLoggerParams.h>
#include <stingraykit/shared_ptr.h>
#include <stingraykit/string/ToString.h>
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
		typedef string_ostream					StreamType;

		typedef void LogFunction(const NamedLoggerParams* loggerParams, LogLevel logLevel, const std::string& message);

		const NamedLoggerParams*				_loggerParams;
		LogLevel								_loggerLogLevel;
		LogLevel								_streamLogLevel;
		shared_ptr<StreamType>					_stream;
		DuplicatingLogsFilter*					_duplicatingLogsFilter;
		shared_ptr<Detail::HideDuplicatingLogs>	_hideDuplicatingLogs;
		LogFunction*							_logFunction;

	public:
		LoggerStream(const NamedLoggerParams* loggerParams, LogLevel loggerLogLevel, LogLevel streamLogLevel, DuplicatingLogsFilter* duplicatingLogsFilter, LogFunction* logFunction);
		~LoggerStream();

		template < typename T >
		typename EnableIf<!IsInt<T>::Value, LoggerStream&>::ValueT operator << (const T& val)
		{
			if (_streamLogLevel < _loggerLogLevel)
				return *this;

			if (!_stream)
				_stream.reset(new StreamType);
			ToString(*_stream, val);
			return *this;
		}

		template < typename T >
		typename EnableIf<IsInt<T>::Value, LoggerStream&>::ValueT operator << (T val)
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
