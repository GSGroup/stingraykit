#ifndef __GS_DVRLIB_TOOLKIT_SORTORDER_H__
#define __GS_DVRLIB_TOOLKIT_SORTORDER_H__


#include <stingray/toolkit/toolkit.h>


namespace stingray
{


	struct SortOrder
	{
		TOOLKIT_ENUM_VALUES
		(
			Ascending,
			Descending
		);

		TOOLKIT_DECLARE_ENUM_CLASS(SortOrder);
	};


}

	
#endif
