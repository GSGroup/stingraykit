#include <stingray/toolkit/EnumToString.h>
#include <stingray/toolkit/exception.h>

namespace stingray
{
	namespace Detail
	{
		void EnumToStringMap_throw(const std::string& msg) { TOOLKIT_THROW(msg); }
	}
}
