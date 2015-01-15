#ifndef STINGRAYKIT_RECT_H
#define STINGRAYKIT_RECT_H


#include <stingraykit/Position.h>


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


	template<typename ValueType_>
	struct BasicRect
	{
		typedef ValueType_ ValueType;

		ValueType X1, Y1, X2, Y2;

		BasicRect() : X1(0), Y1(0), X2(0), Y2(0) { }
		BasicRect(ValueType x1, ValueType y1, ValueType x2, ValueType y2) : X1(x1), Y1(y1), X2(x2), Y2(y2) { }
		BasicRect(ValueType w, ValueType h) : X1(0), Y1(0), X2(w), Y2(h) { }
		BasicRect(Size size) : X1(0), Y1(0), X2(size.Width), Y2(size.Height) { }

		ValueType W() const	{ return X2 - X1; }
		ValueType H() const	{ return Y2 - Y1; }

		BasicRect Move(BasicPosition<ValueType> d) const	{ return BasicRect(X1 + d.X, Y1 + d.Y, X2 + d.X, Y2 + d.Y); }
		BasicRect Move(ValueType dx, ValueType dy) const	{ return BasicRect(X1 + dx, Y1 + dy, X2 + dx, Y2 + dy); }
		BasicPosition<ValueType> GetTopLeft() const			{ return BasicPosition<ValueType>(X1, Y1); }
		BasicPosition<ValueType> GetRightBottom() const		{ return BasicPosition<ValueType>(X2, Y2); }
		Size GetSize() const								{ return Size(W(), H()); }

		BasicRect Intersect(const BasicRect& other) const
		{ return BasicRect(std::max(X1, other.X1), std::max(Y1, other.Y1), std::min(X2, other.X2), std::min(Y2, other.Y2)); }

		std::string ToString() const						{ return StringBuilder() % "(" % GetTopLeft() % ", " % GetRightBottom() % ")"; }
		bool Valid() const									{ return X2 > X1 && Y2 > Y1; }

		bool operator == (const BasicRect& other) const		{ return GetTopLeft() == other.GetTopLeft() && GetRightBottom() == other.GetRightBottom(); }
		bool operator != (const BasicRect& other) const		{ return !((*this) == other); }
	};
	typedef BasicRect<int> Rect;


}


#endif
