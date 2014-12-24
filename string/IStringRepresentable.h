#ifndef STINGRAY_TOOLKIT_STRING_ISTRINGREPRESENTABLE_H
#define STINGRAY_TOOLKIT_STRING_ISTRINGREPRESENTABLE_H

#include <string>

namespace stingray
{


	struct IStringRepresentable
	{
		virtual ~IStringRepresentable() { }
		virtual std::string ToString() const = 0;
	};


}

#endif
