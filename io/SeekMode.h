#ifndef STINGRAY_TOOLKIT_IO_SEEKMODE_H
#define STINGRAY_TOOLKIT_IO_SEEKMODE_H


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
