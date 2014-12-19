#ifndef POSIX_BACKTRACE_H__
#define POSIX_BACKTRACE_H__

#include <string>
#include <vector>

namespace stingray { namespace posix 
{

	class Backtrace
	{
		std::vector<void *> _backtrace;
	public:
		Backtrace();
		std::string Get() const;
		std::string ToString() const { return Get(); }
	};

}}

#endif
