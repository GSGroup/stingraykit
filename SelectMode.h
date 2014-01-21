#ifndef STINGRAY_TOOLKIT_SELECTMODE_H
#define STINGRAY_TOOLKIT_SELECTMODE_H


#include <stingray/toolkit/toolkit.h>


namespace stingray
{

	struct SelectMode
	{
		TOOLKIT_ENUM_VALUES
		(
			Read,
			Write,
			Error
		);

	TOOLKIT_DECLARE_ENUM_CLASS(SelectMode);
};

}


#endif
