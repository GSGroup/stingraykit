#ifndef STINGRAY_TOOLKIT_IO_SEEKMODE_H
#define STINGRAY_TOOLKIT_IO_SEEKMODE_H


#include <stingray/toolkit/toolkit.h>


namespace stingray
{

	struct SeekMode
	{
		STINGRAYKIT_ENUM_VALUES
		(
			Begin,
			End,
			Current
		);

		STINGRAYKIT_DECLARE_ENUM_CLASS(SeekMode);
	};

}


#endif
