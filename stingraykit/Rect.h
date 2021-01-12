#ifndef STINGRAYKIT_RECT_H
#define STINGRAYKIT_RECT_H

// Copyright (c) 2011 - 2019, GS Group, https://github.com/GSGroup
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
		int		Width;
		int		Height;

		Size() : Width(), Height() { }
		Size(int width, int height) : Width(width), Height(height) { }

		Size operator + (const Size& other) const			{ return Size(Width + other.Width, Height + other.Height); }
		Size operator - (const Size& other) const			{ return Size(Width - other.Width, Height - other.Height); }
		Size& operator += (const Size& other)				{ Width += other.Width; Height += other.Height; return *this; }
		Size& operator -= (const Size& other)				{ Width -= other.Width; Height -= other.Height; return *this; }
		Size operator * (int k) const						{ return Size(Width * k, Height * k); }
		Size operator / (int k) const						{ return Size(Width / k, Height / k); }
		Size operator / (const Size& other) const			{ return Size(Width / other.Width, Height / other.Height); }

		bool Valid() const									{ return Width > 0 && Height > 0; }

		bool operator == (const Size& other) const			{ return Width == other.Width && Height == other.Height; }
		STINGRAYKIT_GENERATE_EQUALITY_OPERATORS_FROM_EQUAL(Size);

		std::string ToString() const						{ return StringBuilder() % "(" % Width % ", " % Height % ")"; }

		template < typename OStream >
		void Serialize(OStream& ar) const
		{
			ar.Serialize("w", Width);
			ar.Serialize("h", Height);
		}

		template < typename IStream >
		void Deserialize(IStream& ar)
		{
			ar.Deserialize("w", Width);
			ar.Deserialize("h", Height);
		}
	};


	struct SizeScale
	{
		int		WidthScale;
		int		HeightScale;

		SizeScale() : WidthScale(), HeightScale() { }
		SizeScale(int widthScale, int heightScale) : WidthScale(widthScale), HeightScale(heightScale) { }

		bool Valid() const									{ return WidthScale > 0 && HeightScale > 0; }

		bool operator == (const SizeScale& other) const			{ return WidthScale == other.WidthScale && HeightScale == other.HeightScale; }
		STINGRAYKIT_GENERATE_EQUALITY_OPERATORS_FROM_EQUAL(SizeScale);

		std::string ToString() const						{ return StringBuilder() % "(" % WidthScale % ":" % HeightScale % ")"; }

		template < typename OStream >
		void Serialize(OStream& ar) const
		{
			ar.Serialize("ws", WidthScale);
			ar.Serialize("hs", HeightScale);
		}

		template < typename IStream >
		void Deserialize(IStream& ar)
		{
			ar.Deserialize("ws", WidthScale);
			ar.Deserialize("hs", HeightScale);
		}
	};


	template < typename ValueType_ >
	struct BasicRect
	{
		typedef ValueType_ ValueType;

		ValueType		X1;
		ValueType		Y1;
		ValueType		X2;
		ValueType		Y2;

		BasicRect() : X1(0), Y1(0), X2(0), Y2(0) { }
		BasicRect(ValueType x1, ValueType y1, ValueType x2, ValueType y2) : X1(x1), Y1(y1), X2(x2), Y2(y2) { }
		BasicRect(ValueType w, ValueType h) : X1(0), Y1(0), X2(w), Y2(h) { }
		BasicRect(Size size) : X1(0), Y1(0), X2(size.Width), Y2(size.Height) { }

		ValueType GetWidth() const	{ return X2 - X1; }
		ValueType GetHeight() const	{ return Y2 - Y1; }

		BasicRect Move(BasicPosition<ValueType> d) const	{ return BasicRect(X1 + d.X, Y1 + d.Y, X2 + d.X, Y2 + d.Y); }
		BasicRect Move(ValueType dx, ValueType dy) const	{ return BasicRect(X1 + dx, Y1 + dy, X2 + dx, Y2 + dy); }

		BasicPosition<ValueType> GetTopLeft() const			{ return BasicPosition<ValueType>(X1, Y1); }
		BasicPosition<ValueType> GetRightBottom() const		{ return BasicPosition<ValueType>(X2, Y2); }

		Size GetSize() const								{ return Size(GetWidth(), GetHeight()); }

		bool Intersects(const BasicRect& other) const
		{ return X1 <= other.X2 && X2 >= other.X1 && Y1 <= other.Y2 && Y2 >= other.Y1; }

		BasicRect Intersect(const BasicRect& other) const
		{ return BasicRect(std::max(X1, other.X1), std::max(Y1, other.Y1), std::min(X2, other.X2), std::min(Y2, other.Y2)); }

		std::string ToString() const						{ return StringBuilder() % "(" % GetTopLeft() % ", " % GetRightBottom() % ")"; }
		bool Valid() const									{ return X2 > X1 && Y2 > Y1; }

		bool operator == (const BasicRect& other) const		{ return GetTopLeft() == other.GetTopLeft() && GetRightBottom() == other.GetRightBottom(); }
		STINGRAYKIT_GENERATE_EQUALITY_OPERATORS_FROM_EQUAL(BasicRect);

		template < typename OStream >
		void Serialize(OStream& ar) const
		{
			ar.Serialize("x1", X1);
			ar.Serialize("y1", Y1);
			ar.Serialize("x2", X2);
			ar.Serialize("y2", Y2);
		}

		template < typename IStream >
		void Deserialize(IStream& ar)
		{
			ar.Deserialize("x1", X1);
			ar.Deserialize("y1", Y1);
			ar.Deserialize("x2", X2);
			ar.Deserialize("y2", Y2);
		}
	};
	typedef BasicRect<int> Rect;

}

#endif
