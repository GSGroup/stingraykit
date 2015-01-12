#ifndef STINGRAY_TOOLKIT_LOG_ILOGGERSINK_H
#define STINGRAY_TOOLKIT_LOG_ILOGGERSINK_H


#include <stingray/toolkit/log/LoggerMessage.h>
#include <stingray/toolkit/shared_ptr.h>


namespace stingray
{

	/**
	 * @addtogroup core_log
	 * @{
	 */

	struct ILoggerSink
	{
		virtual ~ILoggerSink() { }

		virtual void Log(const LoggerMessage& message) = 0;
	};
	STINGRAYKIT_DECLARE_PTR(ILoggerSink);
	STINGRAYKIT_DECLARE_ENUMERABLE(ILoggerSinkPtr);

	/** @} */

}


#endif
