#include <stingraykit/signal/signal_policies.h>
#include <stingraykit/log/Logger.h>

namespace stingray
{

	void signal_policies::exception_handling::DefaultSignalExceptionHandler(const std::exception &ex)
	{ Logger::Error() << "Uncaught exception in signal handler: " << diagnostic_information(ex); }

}
