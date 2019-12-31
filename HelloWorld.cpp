#include <stingraykit/log/Logger.h>

using namespace stingray;


int main(int argc, char** argv)
{
	try
	{
		Thread::SetCurrentThreadName("main");

		Logger::Info() << "Hello world!";

		return 0;
	}
	catch (const std::exception& ex)
	{ Logger::Error() << "Application exited due to: " << ex; }
	catch (...)
	{ Logger::Error() << "unknown exception!"; }
}
