#include <stingray/toolkit/TypeInfo.h>



namespace stingray
{

	std::string TypeInfo::GetName() const
	{
		return Demangle(GetRawName());
	}


	std::string TypeInfo::GetNamespaceName() const
	{
		std::string name = GetName();
		size_t template_start = name.find('<');
		std::string delim = "::";
		size_t delim_pos = name.rfind(delim, template_start);
		return delim_pos == std::string::npos ? "" : std::string(name, 0, delim_pos);
	}


	std::string TypeInfo::GetClassName() const
	{
		std::string name = GetName();
		size_t template_start = name.find('<');
		std::string delim = "::";
		size_t delim_pos = name.rfind(delim, template_start);
		return delim_pos == std::string::npos ? name : name.substr(delim_pos + delim.size());
	}


	std::string TypeInfo::ToString() const
	{
		return GetName();
	}

}
