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


	LoggerStream::LoggerStream(const char* loggerName, LogLevel loggerLogLevel, LogLevel streamLogLevel, DuplicatingLogsFilter* duplicatingLogsFilter, LogFunction* logFunction) :
		_loggerName(loggerName), _loggerLogLevel(loggerLogLevel), _streamLogLevel(streamLogLevel), _duplicatingLogsFilter(duplicatingLogsFilter), _logFunction(logFunction)
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
	{ _logFunction(_loggerName, _streamLogLevel, message); }

}
