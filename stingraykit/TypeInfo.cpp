// Copyright (c) 2011 - 2019, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/TypeInfo.h>

#include <stingraykit/toolkit.h>

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
