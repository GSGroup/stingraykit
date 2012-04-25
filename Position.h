#ifndef __GS_STINGRAY_TOOLKIT_POSITION_H__
#define __GS_STINGRAY_TOOLKIT_POSITION_H__


#include <stingray/toolkit/StringUtils.h>


namespace stingray
{

	struct Position
	{
		int X, Y;

		Position() : X(0), Y(0) { }
		Position(int x, int y) : X(x), Y(y) { }

		std::string ToString() const { return StringBuilder() % "(" % X % ", " % Y % ")"; }
	};

}


#endif
