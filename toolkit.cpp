#include <stingray/toolkit/toolkit.h>
#include <stingray/toolkit/StringUtils.h>

#include <stingray/log/Logger.h>

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

}
