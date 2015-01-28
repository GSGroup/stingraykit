#include <stingraykit/ProgressValue.h>
#include <stingraykit/string/ToString.h>

namespace stingray
{

	std::string ProgressValue::ToString() const
	{ return StringBuilder() % InPercents() % "%"; }

}
