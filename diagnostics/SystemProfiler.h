#ifndef STINGRAY_TOOLKIT_DIAGNOSTICS_SYSTEMPROFILER_H
#define STINGRAY_TOOLKIT_DIAGNOSTICS_SYSTEMPROFILER_H


#include <memory>
#include <string>
#include <stingray/toolkit/toolkit.h>


namespace stingray
{

	/**
	 * @addtogroup toolkit_threads
	 * @{
	 */

	class SystemProfiler
	{
		TOOLKIT_NONCOPYABLE(SystemProfiler);

		class Impl;

		std::auto_ptr<Impl>		_impl;
		std::string				_message;
		s64						_thresholdMs;
		s64						_criticalMs;
		s64						_start;

	public:
		SystemProfiler(const std::string& message, s64 thresholdMs, s64 criticalMs = 0);
		~SystemProfiler();
	};

	/** @} */

}


#endif
