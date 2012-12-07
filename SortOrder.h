#ifndef STINGRAY_TOOLKIT_SORTORDER_H
#define STINGRAY_TOOLKIT_SORTORDER_H


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
