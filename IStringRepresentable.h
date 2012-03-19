#ifndef __GS_DVRLIB_TOOLKIT_ISTRINGREPRESENTABLE_H__
#define __GS_DVRLIB_TOOLKIT_ISTRINGREPRESENTABLE_H__

#include <string>

namespace dvrlib
{


	struct IStringRepresentable
	{
		virtual ~IStringRepresentable() { }
		virtual std::string ToString() const = 0;
	};


}

#endif
