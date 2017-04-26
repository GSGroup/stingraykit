// Copyright (c) 2011 - 2017, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/log/LoggerStream.h>

#include <cstring>

namespace stingray
{

	namespace Detail
	{
		bool HideDuplicatingLogs::KeyType::operator < (const KeyType& other) const
		{
			if (Line < other.Line)
				return true;
			return strcmp(Filename, other.Filename) < 0;
		}
	}


	LoggerStream::LoggerStream(const NamedLoggerParams* loggerParams, LogLevel loggerLogLevel, LogLevel streamLogLevel, DuplicatingLogsFilter* duplicatingLogsFilter, LogFunction* logFunction) :
		_loggerParams(loggerParams), _loggerLogLevel(loggerLogLevel), _streamLogLevel(streamLogLevel), _duplicatingLogsFilter(duplicatingLogsFilter), _logFunction(logFunction)
	{ }


	LoggerStream::~LoggerStream()
	{
		if (!_stream || !_stream.unique())
			return;

		if (_streamLogLevel >= _loggerLogLevel)
		{
			if (std::uncaught_exception())
			{
				try
				{ DoLog(_stream->str()); }
				catch(const std::exception &ex)
				{ return; }
			}
			else
				DoLog(_stream->str());
		}
	}


	void LoggerStream::DoLog(const std::string& message)
	{
		if (_duplicatingLogsFilter && _hideDuplicatingLogs)
		{
			DuplicatingLogsFilter::StringCounter* str_cnt = _duplicatingLogsFilter->Get(_hideDuplicatingLogs->Key);
			if (str_cnt->Count < 0)
			{
				DoLogImpl(message); // Displaying first message
				str_cnt->Reset(message);
			}
			else if (str_cnt->Count >= 0 && str_cnt->Str != message)
			{
				if (str_cnt->Count > 0)
					DoLogImpl(StringBuilder() % str_cnt->Str % " (" % str_cnt->Count % " times)");
				DoLogImpl(message); // Displaying first message
				str_cnt->Reset(message);
			}
			else if (str_cnt->Count >= _hideDuplicatingLogs->Count && str_cnt->Str == message)
			{
				if (str_cnt->Count > 0)
					DoLogImpl(StringBuilder() % str_cnt->Str % " (" % str_cnt->Count % " times)");
				str_cnt->Reset(message);
			}
			else
				++str_cnt->Count;
		}
		else
			DoLogImpl(message);
	}


	void LoggerStream::DoLogImpl(const std::string& message)
	{ _logFunction(_loggerParams, _streamLogLevel, message); }

}
