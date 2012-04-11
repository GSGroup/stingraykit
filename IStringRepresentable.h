#ifndef __GS_DVRLIB_TOOLKIT_ISTRINGREPRESENTABLE_H__
#define __GS_DVRLIB_TOOLKIT_ISTRINGREPRESENTABLE_H__

#include <string>

#include <stingray/toolkit/NestedTypeCheck.h>

namespace stingray
{


	TOOLKIT_DECLARE_METHOD_CHECK(ToString);

	struct IStringRepresentable
	{
		virtual ~IStringRepresentable() { }
		virtual std::string ToString() const = 0;
	};


}

#endif
