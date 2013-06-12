#include <stingray/toolkit/SoftwareVersion.h>

#include <cstdio>


namespace stingray
{

	SoftwareVersion SoftwareVersion::FromString(const std::string& version)
	{
		unsigned major, minor, build;
		TOOLKIT_CHECK(std::sscanf("%u.%u.%u", version.c_str(), &major, &minor, &build) == 3, FormatException());
		return SoftwareVersion(major, minor, build);
	}

}
