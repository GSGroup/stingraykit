#include <stingraykit/ProgressValue.h>
#include <stingraykit/string/StringUtils.h>

namespace stingray
{

	std::string ProgressValue::ToString() const
	{ return StringBuilder() % InPercents() % "%"; }

}
