#ifndef STINGRAYKIT_STRING_ISTRINGREPRESENTABLE_H
#define STINGRAYKIT_STRING_ISTRINGREPRESENTABLE_H

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
