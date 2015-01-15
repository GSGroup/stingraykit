#include <stingraykit/self_counter.h>

#include <stingraykit/fatal.h>
#include <stingraykit/string/StringUtils.h>

namespace stingray {

	namespace Detail
	{
		void SelfCounterHelper::CheckAddRef(int count)
		{
			if (count <= 0)
				STINGRAYKIT_FATAL(StringBuilder() % "self_counter add_ref value error: " % count);
		}
		void SelfCounterHelper::CheckReleaseRef(int count)
		{
			if (count < 0)
				STINGRAYKIT_FATAL(StringBuilder() % "self_counter release_ref value error: " % count);
		}
	}


}


