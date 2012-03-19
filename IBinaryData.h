#ifndef __GS_DVRLIB_TOOLKIT_IBINARYDATA_H__
#define __GS_DVRLIB_TOOLKIT_IBINARYDATA_H__

#include <vector>

#include <stingray/toolkit/shared_ptr.h>

namespace stingray
{

	struct IBinaryData
	{
		virtual ~IBinaryData() {}

		virtual void Read(size_t offset, u8* data, size_t count) const = 0;

		virtual size_t GetLength() const = 0;
	};
	TOOLKIT_DECLARE_PTR(IBinaryData);

}

#endif
