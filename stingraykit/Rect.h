#ifndef STINGRAYKIT_RECT_H
#define STINGRAYKIT_RECT_H

// Copyright (c) 2011 - 2015, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.


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

		inline bool operator==(const Size &other) const		{ return Width == other.Width && Height == other.Height; }
		inline bool operator!=(const Size &other) const		{ return !((*this) == other); }

		std::string ToString() const						{ return StringBuilder() % "(" % Width % ", " % Height % ")"; }

		template < typename OStream >
		void Serialize(OStream& ar) const
		{ ar.Serialize("w", Width).Serialize("h", Height); }

		template < typename IStream >
		void Deserialize(IStream& ar)
		{ ar.Deserialize("w", Width).Deserialize("h", Height); }
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

		inline bool Intersects(const BasicRect &other) const
		{ return X1 <= other.X2 && X2 >= other.X1 && Y1 <= other.Y2 && Y2 >= other.Y1; }

		BasicRect Intersect(const BasicRect& other) const
		{ return BasicRect(std::max(X1, other.X1), std::max(Y1, other.Y1), std::min(X2, other.X2), std::min(Y2, other.Y2)); }

		std::string ToString() const						{ return StringBuilder() % "(" % GetTopLeft() % ", " % GetRightBottom() % ")"; }
		bool Valid() const									{ return X2 > X1 && Y2 > Y1; }

		bool operator == (const BasicRect& other) const		{ return GetTopLeft() == other.GetTopLeft() && GetRightBottom() == other.GetRightBottom(); }
		bool operator != (const BasicRect& other) const		{ return !((*this) == other); }

		template < typename OStream >
		void Serialize(OStream& ar) const
		{ ar.Serialize("x1", X1).Serialize("y1", Y1).Serialize("x2", X2).Serialize("y2", Y2); }

		template < typename IStream >
		void Deserialize(IStream& ar)
		{ ar.Deserialize("x1", X1).Deserialize("y1", Y1).Deserialize("x2", X2).Deserialize("y2", Y2); }
	};
	typedef BasicRect<int> Rect;


}


#endif
