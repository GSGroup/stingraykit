#ifndef STINGRAY_TOOLKIT_POSITION_H
#define STINGRAY_TOOLKIT_POSITION_H


#include <stingray/toolkit/StringUtils.h>


namespace stingray
{


	template<typename ValueType>
	struct BasicPosition
	{
		ValueType X, Y;

		BasicPosition() : X(0), Y(0) { }
		BasicPosition(ValueType x, ValueType y) : X(x), Y(y) { }

		std::string ToString() const						{ return StringBuilder() % "(" % X % ", " % Y % ")"; }

		bool operator == (const BasicPosition& other) const	{ return X == other.X && Y == other.Y; }
		bool operator != (const BasicPosition& other) const	{ return !((*this) == other); }
	};
	typedef BasicPosition<int> Position;


}


#endif
