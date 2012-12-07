#ifndef STINGRAY_TOOLKIT_RECT_H
#define STINGRAY_TOOLKIT_RECT_H


#include <stingray/toolkit/Position.h>


namespace stingray
{

	struct Size
	{
		int Width, Height;
		inline Size(): Width(), Height() {}
		inline Size(int w, int h): Width(w), Height(h) {}

		inline Size operator + (const Size& other) const	{ return Size(Width + other.Width, Height + other.Height); }
		inline Size operator - (const Size& other) const	{ return Size(Width - other.Width, Height - other.Height); }
		inline Size& operator += (const Size& other)		{ Width += other.Width; Height += other.Height; return *this; }
		inline Size& operator -= (const Size& other)		{ Width -= other.Width; Height -= other.Height; return *this; }
		inline Size operator * (int k)						{ return Size(Width * k, Height * k); }
		inline Size operator / (int k)						{ return Size(Width / k, Height / k); }
		inline bool Valid() const							{ return Width > 0 && Height > 0; }

		inline bool operator==(const Size &other) const	{ return Width == other.Width && Height == other.Height; }
		inline bool operator!=(const Size &other) const { return !((*this) == other); }
	};

	struct Rect
	{
		int X1, Y1, X2, Y2;

		Rect() : X1(0), Y1(0), X2(0), Y2(0) { }
		Rect(int x1, int y1, int x2, int y2) : X1(x1), Y1(y1), X2(x2), Y2(y2) { }
		Rect(int w, int h) : X1(0), Y1(0), X2(w), Y2(h) { }

		int W() const	{ return X2 - X1; }
		int H() const	{ return Y2 - Y1; }

		Rect Move(Position d) const { return Rect(X1 + d.X, Y1 + d.Y, X2 + d.X, Y2 + d.Y); }
		Rect Move(int dx, int dy) const { return Rect(X1 + dx, Y1 + dy, X2 + dx, Y2 + dy); }
		Position GetTopLeft() const { return Position(X1, Y1); }
		Position GetRightBottom() const { return Position(X2, Y2); }

		Rect Intersect(const Rect& other) const
		{ return Rect(std::max(X1, other.X1), std::max(Y1, other.Y1), std::min(X2, other.X2), std::min(Y2, other.Y2)); }

		std::string ToString() const { return StringBuilder() % "(" % GetTopLeft() % ", " % GetRightBottom() % ")"; }
	};

}


#endif
