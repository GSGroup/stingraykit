#ifndef STINGRAYKIT_LOG_LOGLEVEL_H
#define STINGRAYKIT_LOG_LOGLEVEL_H


#include <stingraykit/toolkit.h>


namespace stingray
{

	/**
	 * @addtogroup toolkit_log
	 * @{
	 */

	struct LogLevel
	{
		STINGRAYKIT_ENUM_VALUES
		(
			Trace,
			Debug,
			Info,
			Warning,
			Error
		);

		STINGRAYKIT_DECLARE_ENUM_CLASS(LogLevel);
	};

	/** @} */

}


#endif
