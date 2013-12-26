#include <stingray/toolkit/TypeInfo.h>

#include <stingray/toolkit/toolkit.h>


namespace stingray
{

	std::string TypeInfo::GetName() const
	{
		return Demangle(GetRawName());
	}


	std::string TypeInfo::GetNamespaceName() const
	{
		std::string name = GetName();
		size_t delim = name.find_last_of("::");
		return delim == std::string::npos ? "" : std::string(name, 0, delim);
	}


	std::string TypeInfo::GetClassName() const
	{
		std::string name = GetName();
		size_t delim = name.find_last_of("::");
		return delim == std::string::npos ? name : std::string(name, delim + 1);
	}


	std::string TypeInfo::ToString() const
	{
		return GetName();
	}

}
