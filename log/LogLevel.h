#ifndef STINGRAY_TOOLKIT_LOG_LOGLEVEL_H
#define STINGRAY_TOOLKIT_LOG_LOGLEVEL_H


#include <stingray/toolkit/toolkit.h>


namespace stingray
{

	/**
	 * @addtogroup toolkit_log
	 * @{
	 */

	struct LogLevel
	{
		TOOLKIT_ENUM_VALUES
		(
			Trace,
			Debug,
			Info,
			Warning,
			Error
		);

		TOOLKIT_DECLARE_ENUM_CLASS(LogLevel);
	};

	/** @} */

}


#endif
