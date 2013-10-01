#include <stingray/toolkit/ProgressValue.h>
#include <stingray/toolkit/StringUtils.h>

namespace stingray
{

	std::string ProgressValue::ToString() const
	{ return StringBuilder() % InPercents() % "%"; }

}
