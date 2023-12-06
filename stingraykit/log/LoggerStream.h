#ifndef STINGRAYKIT_LOG_LOGGERSTREAM_H
#define STINGRAYKIT_LOG_LOGGERSTREAM_H

// Copyright (c) 2011 - 2022, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/log/LogLevel.h>
#include <stingraykit/log/NamedLoggerParams.h>
#include <stingraykit/string/ToString.h>
#include <stingraykit/time/ElapsedTime.h>

#include <stingraykit/unique_ptr.h>

#include <map>

namespace stingray
{

	class DuplicatingLogsFilter;

	namespace Detail
	{
		struct HideDuplicatingLogs
		{
			struct KeyType
			{
				const char* const						Filename;
				const int								Line;

				KeyType(const char* filename, int line) : Filename(filename), Line(line) { }

				bool operator < (const KeyType& other) const;
			};

			const unsigned								Count;
			const optional<TimeDuration>				Interval;
			const KeyType								Key;
			DuplicatingLogsFilter*						Filter;

			HideDuplicatingLogs(unsigned count, const char* filename, int line)
				: Count(count), Key(filename, line), Filter(NULL)
			{ STINGRAYKIT_CHECK(Count > 0, ArgumentException("count")); }

			HideDuplicatingLogs(TimeDuration interval, const char* filename, int line)
				: Count(std::numeric_limits<unsigned>::max()), Interval(interval), Key(filename, line), Filter(NULL)
			{ STINGRAYKIT_CHECK(Interval > TimeDuration(), ArgumentException("interval", Interval)); }

			HideDuplicatingLogs(unsigned count, const char* filename, int line, DuplicatingLogsFilter& filter)
				: Count(count), Key(filename, line), Filter(&filter)
			{ STINGRAYKIT_CHECK(Count > 0, ArgumentException("count")); }

			HideDuplicatingLogs(TimeDuration interval, const char* filename, int line, DuplicatingLogsFilter& filter)
				: Count(std::numeric_limits<unsigned>::max()), Interval(interval), Key(filename, line), Filter(&filter)
			{ STINGRAYKIT_CHECK(Interval > TimeDuration(), ArgumentException("interval", Interval)); }
		};
	}

	class DuplicatingLogsFilter
	{
	public:
		struct StringCounter
		{
			std::string									Str;
			optional<unsigned>							Count;
			optional<ElapsedTime>						Elapsed;

			StringCounter(const std::string& str = "") : Str(str) { }

			void Reset(const std::string& str, bool hasInterval)
			{
				Str = str;
				Count = 0;

				if (hasInterval)
					Elapsed.emplace();
				else
					Elapsed.reset();
			}
		};

	private:
		using KeyType = Detail::HideDuplicatingLogs::KeyType;
		using LastMessagesMap = std::map<KeyType, StringCounter>;

	private:
		LastMessagesMap									_lastMessages;

	public:
		StringCounter* Get(const KeyType& key) { return &_lastMessages[key]; }
	};


#define STINGRAYKIT_HIDE_DUPLICATING_LOGS(CountOrInterval, ...) stingray::Detail::HideDuplicatingLogs(CountOrInterval, __FILE__, __LINE__, ##__VA_ARGS__)


	class LoggerStream
	{
		STINGRAYKIT_NONCOPYABLE(LoggerStream);
		STINGRAYKIT_DEFAULTMOVABLE(LoggerStream);

	public:
		using LogFunction = void (const NamedLoggerParams* loggerParams, LogLevel logLevel, const std::string& message);

	private:
		const NamedLoggerParams*						_loggerParams;
		LogLevel										_loggerLogLevel;
		LogLevel										_streamLogLevel;
		unique_ptr<string_ostream>						_stream;
		DuplicatingLogsFilter*							_duplicatingLogsFilter;
		unique_ptr<Detail::HideDuplicatingLogs>			_hideDuplicatingLogs;
		LogFunction*									_logFunction;

	public:
		LoggerStream(const NamedLoggerParams* loggerParams, LogLevel loggerLogLevel, LogLevel streamLogLevel, DuplicatingLogsFilter* duplicatingLogsFilter, LogFunction* logFunction);
		~LoggerStream();

		template < typename T >
		typename EnableIf<!IsInt<T>::Value, LoggerStream&>::ValueT operator << (const T& val)
		{
			if (_streamLogLevel < _loggerLogLevel)
				return *this;

			if (!_stream)
				_stream = make_unique_ptr<string_ostream>();
			ToString(*_stream, val);
			return *this;
		}

		template < typename T >
		typename EnableIf<IsInt<T>::Value, LoggerStream&>::ValueT operator << (T val)
		{
			if (_streamLogLevel < _loggerLogLevel)
				return *this;

			if (!_stream)
				_stream = make_unique_ptr<string_ostream>();
			ToString(*_stream, val);
			return *this;
		}

		LoggerStream& operator << (const Detail::HideDuplicatingLogs& val)
		{
			if (val.Filter)
				_duplicatingLogsFilter = val.Filter;

			if (_duplicatingLogsFilter)
				_hideDuplicatingLogs = make_unique_ptr<Detail::HideDuplicatingLogs>(val);

			return *this;
		}

	private:
		void DoLog(const std::string& message);
		void DoLogImpl(const std::string& message);
	};

}

#endif
