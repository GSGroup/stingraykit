#ifndef STINGRAYKIT_LOG_ILOGGERSINK_H
#define STINGRAYKIT_LOG_ILOGGERSINK_H


#include <stingraykit/log/LoggerMessage.h>
#include <stingraykit/shared_ptr.h>


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

	/** @} */

}


#endif
