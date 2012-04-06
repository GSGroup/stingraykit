#ifndef __GS_STINGRAY_TOOLKIT_POSITION_H__
#define __GS_STINGRAY_TOOLKIT_POSITION_H__



namespace stingray
{

	struct Position
	{
		int X, Y;

		Position() : X(0), Y(0) { }
		Position(int x, int y) : X(x), Y(y) { }
	};

}


#endif
