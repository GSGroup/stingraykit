// Copyright (c) 2011 - 2024, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/log/LoggerStream.h>

#include <string.h>

namespace stingray
{

	namespace Detail
	{
		bool HideDuplicatingLogs::KeyType::operator < (const KeyType& other) const
		{
			if (Line != other.Line)
				return Line < other.Line;
			return strcmp(Filename, other.Filename) < 0;
		}
	}


	LoggerStream::LoggerStream(const NamedLoggerParams* loggerParams, LogLevel loggerLogLevel, LogLevel streamLogLevel, DuplicatingLogsFilter* duplicatingLogsFilter, LogFunction* logFunction)
		: _loggerParams(loggerParams), _loggerLogLevel(loggerLogLevel), _streamLogLevel(streamLogLevel), _duplicatingLogsFilter(duplicatingLogsFilter), _logFunction(logFunction)
	{ }


	LoggerStream::~LoggerStream()
	{
		if (!_stream)
			return;

		try
		{ DoLog(_stream->str()); }
		catch (const std::exception&)
		{ }
	}


	void LoggerStream::DoLog(const std::string& message)
	{
		if (!_duplicatingLogsFilter || !_hideDuplicatingLogs)
		{
			DoLogImpl(message);
			return;
		}

		DuplicatingLogsFilter::StringCounter* stringCounter = _duplicatingLogsFilter->Get(_hideDuplicatingLogs->Key);

		if (!stringCounter->Count)
		{
			DoLogImpl(message); // Displaying first message
			stringCounter->Reset(message, _hideDuplicatingLogs->Interval.is_initialized());
			return;
		}

		const bool newMessage = stringCounter->Str != message;

		if (newMessage
				|| stringCounter->Count >= _hideDuplicatingLogs->Count
				|| (stringCounter->Elapsed && stringCounter->Elapsed->Elapsed() >= _hideDuplicatingLogs->Interval))
		{
			if (stringCounter->Count > 0u)
				DoLogImpl(StringBuilder() % stringCounter->Str % " (" % stringCounter->Count % " times)");

			if (newMessage)
				DoLogImpl(message); // Displaying first message

			stringCounter->Reset(message, _hideDuplicatingLogs->Interval.is_initialized());
		}
		else
			++*stringCounter->Count;
	}


	void LoggerStream::DoLogImpl(const std::string& message)
	{ _logFunction(_loggerParams, _streamLogLevel, message); }

}
