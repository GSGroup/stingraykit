#include <stingray/toolkit/signal_policies.h>
#include <stingray/toolkit/log/Logger.h>

namespace stingray
{

	void signal_policies::exception_handling::DefaultSignalExceptionHandler(const std::exception &ex)
	{ Logger::Error() << "Uncaught exception in signal handler: " << diagnostic_information(ex); }

}
