#ifndef __GS_STINGRAY_TOOLKIT_SEEKMODE_H__
#define __GS_STINGRAY_TOOLKIT_SEEKMODE_H__


#include <stingray/toolkit/toolkit.h>


namespace stingray
{

	struct SeekMode
	{
		TOOLKIT_ENUM_VALUES
		(
			Begin,
			End,
			Current
		);

		TOOLKIT_DECLARE_ENUM_CLASS(SeekMode);
	};

}


#endif
