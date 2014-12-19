#ifndef STINGRAY_TOOLKIT_DIAGNOSTICS_EXECUTORSPROFILER_H
#define STINGRAY_TOOLKIT_DIAGNOSTICS_EXECUTORSPROFILER_H


#include <stingray/toolkit/diagnostics/AsyncProfiler.h>
#include <stingray/toolkit/PhoenixSingleton.h>


namespace stingray
{

	/**
	 * @addtogroup toolkit_profiling
	 * @{
	 */

	class ExecutorsProfiler : public PhoenixSingleton<ExecutorsProfiler>
	{
		TOOLKIT_PHOENIXSINGLETON(ExecutorsProfiler);

	private:
		AsyncProfilerPtr	_profiler;

	private:
		ExecutorsProfiler()
			: _profiler(new AsyncProfiler("executorsProfiler"))
		{ }

	public:
		AsyncProfilerWeakPtr GetProfiler() const { return _profiler; }
	};

	/** @} */

}

#endif
