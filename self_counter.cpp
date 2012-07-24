#include <stingray/toolkit/self_counter.h>

#include <stingray/toolkit/fatal.h>
#include <stingray/toolkit/StringUtils.h>

namespace stingray {

	namespace Detail
	{
		void SelfCounterHelper::CheckAddRef(int count)
		{
			if (count <= 0)
				TOOLKIT_FATAL(StringBuilder() % "self_counter add_ref value error: " % count);
		}
		void SelfCounterHelper::CheckReleaseRef(int count)
		{
			if (count < 0)
				TOOLKIT_FATAL(StringBuilder() % "self_counter release_ref value error: " % count);
		}
	}


}


