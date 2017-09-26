#ifndef STINGRAYKIT_STRING_HUMANREADABLESIZE_H
#define STINGRAYKIT_STRING_HUMANREADABLESIZE_H

#include <stingraykit/Types.h>

#include <string>

namespace stingray
{

	std::string ToHumanReadableSize(u64 size, float allowedErrorFactor = 0.01, bool whitespaceBeforeSuffix = false);

	u64 FromHumanReadableSize(const std::string& str);

}

#endif
