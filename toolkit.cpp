#include <stingray/toolkit/toolkit.h>

#include <stdio.h>
#include <stdarg.h>

#include <stingray/log/Logger.h>
#include <stingray/toolkit/Backtrace.h>
#include <stingray/toolkit/StringUtils.h>
#include <stingray/toolkit/array.h>

namespace stingray
{

	NullPtrType null;

	void _append_extended_diagnostics(std::stringstream& result, const Detail::IToolkitException& tkit_ex)
	{
		std::string backtrace = tkit_ex.GetBacktrace();
		result << "\n  in function '" << tkit_ex.GetFunctionName() << "'" <<
			"\n  in file '" << tkit_ex.GetFilename() << "' at line " << ToString(tkit_ex.GetLine());

		if (!backtrace.empty())
			result << "\n" << backtrace;
	}

	void DebuggingHelper::BreakpointHere()
	{
		TRACER;
	}

	void DebuggingHelper::TerminateWithMessage(const std::string& str)
	{
		Logger::Error() << "Terminate was requested: " << str;
		std::terminate();
	}

}

#ifdef _STLP_DEBUG_MESSAGE
	void __stl_debug_message(const char * format_str, ...)
	{
		va_list args;
		va_start(args, format_str);

		stingray::array<char, 4096> buffer;
		int count = vsnprintf(buffer.data(), buffer.size(), format_str, args);
		if (count > 0)
		{
			std::string str(buffer.data(), count);
			stingray::Logger::Error() << str << stingray::Backtrace().Get();
		}
		else
			stingray::Logger::Error() << "Can't form stlport error message!\n" << stingray::Backtrace().Get();
		va_end(args);
	}
#endif


