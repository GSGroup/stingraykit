#ifndef STINGRAY_TOOLKIT_IO_SELECTMODE_H
#define STINGRAY_TOOLKIT_IO_SELECTMODE_H


#include <stingray/toolkit/toolkit.h>


namespace stingray
{

	struct SelectMode
	{
		STINGRAYKIT_ENUM_VALUES(None = 0, Read = 1, Write = 2, Error = 4);
		STINGRAYKIT_DECLARE_ENUM_CLASS(SelectMode);
	};
	STINGRAYKIT_DECLARE_ENUM_CLASS_BIT_OPERATORS(SelectMode);

}


#endif
