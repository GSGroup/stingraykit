#include <stingray/toolkit/Version.h>

#include <cstdio>


namespace stingray
{

	Version Version::FromString(const std::string& version)
	{
		unsigned major, minor, build;
		TOOLKIT_CHECK(std::sscanf(version.c_str(), "%u.%u.%u", &major, &minor, &build) == 3, FormatException(version));
		return Version(major, minor, build);
	}

}
