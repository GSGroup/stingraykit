#include <stingray/toolkit/function/function_name_getter.h>


namespace stingray {
namespace Detail
{
	std::string FuncPtrToString(intptr_t* ptrptr, size_t n)
	{
		char buf[32];
		int size = 0;
		if (n > 1) n = 1; // =(((
		for (size_t i = 0; i < n; ++i)
			size += snprintf(buf + size, sizeof(buf) - size, "0x%08lx", (long)ptrptr[i]);
		return buf;
	}
}}
