#ifndef STINGRAYKIT_IO_SEEKMODE_H
#define STINGRAYKIT_IO_SEEKMODE_H


#include <stingraykit/toolkit.h>


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
