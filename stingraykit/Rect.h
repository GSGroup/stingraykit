#ifndef STINGRAYKIT_RECT_H
#define STINGRAYKIT_RECT_H

// Copyright (c) 2011 - 2024, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/compare/MemberListComparer.h>
#include <stingraykit/string/ToString.h>

namespace stingray
{

	struct Size
	{
		int		Width;
		int		Height;

		Size() : Width(), Height() { }
		Size(int width, int height) : Width(width), Height(height) { }

		bool Valid() const										{ return Width > 0 && Height > 0; }

		bool operator < (const Size& other) const
		{ return CompareMembersLess(&Size::Width, &Size::Height)(*this, other); }
		STINGRAYKIT_GENERATE_COMPARISON_OPERATORS_FROM_LESS(Size);

		Size operator + (const Size& other) const				{ return Size(Width + other.Width, Height + other.Height); }
		Size operator - (const Size& other) const				{ return Size(Width - other.Width, Height - other.Height); }

		Size& operator += (const Size& other)
		{
			Width += other.Width;
			Height += other.Height;
			return *this;
		}

		Size& operator -= (const Size& other)
		{
			Width -= other.Width;
			Height -= other.Height;
			return *this;
		}

		Size operator * (int k) const							{ return Size(Width * k, Height * k); }
		Size operator / (int k) const							{ return Size(Width / k, Height / k); }
		Size operator / (const Size& other) const				{ return Size(Width / other.Width, Height / other.Height); }

		std::string ToString() const							{ return StringBuilder() % Width % "x" % Height; }

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

		bool Valid() const										{ return WidthScale > 0 && HeightScale > 0; }

		bool operator < (const SizeScale& other) const
		{ return CompareMembersLess(&SizeScale::WidthScale, &SizeScale::HeightScale)(*this, other); }
		STINGRAYKIT_GENERATE_COMPARISON_OPERATORS_FROM_LESS(SizeScale);

		std::string ToString() const							{ return StringBuilder() % WidthScale % ":" % HeightScale; }

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


	template < typename ValueType >
	struct BasicPosition
	{
		ValueType		X;
		ValueType		Y;

		BasicPosition() : X(0), Y(0) { }
		BasicPosition(ValueType x, ValueType y) : X(x), Y(y) { }

		bool operator < (const BasicPosition& other) const
		{ return CompareMembersLess(&BasicPosition::X, &BasicPosition::Y)(*this, other); }
		STINGRAYKIT_GENERATE_COMPARISON_OPERATORS_FROM_LESS(BasicPosition);

		std::string ToString() const							{ return StringBuilder() % "[" % X % ", " % Y % "]"; }

		template < typename OStream >
		void Serialize(OStream& ar) const
		{
			ar.Serialize("x", X);
			ar.Serialize("y", Y);
		}

		template < typename IStream >
		void Deserialize(IStream& ar)
		{
			ar.Deserialize("x", X);
			ar.Deserialize("y", Y);
		}
	};
	using Position = BasicPosition<int>;


	template < typename ValueType_ >
	struct BasicRect
	{
		using ValueType = ValueType_;

		ValueType		X1;
		ValueType		Y1;
		ValueType		X2;
		ValueType		Y2;

		BasicRect() : X1(0), Y1(0), X2(0), Y2(0) { }
		BasicRect(Size size) : X1(0), Y1(0), X2(size.Width), Y2(size.Height) { }
		BasicRect(ValueType w, ValueType h) : X1(0), Y1(0), X2(w), Y2(h) { }
		BasicRect(ValueType x1, ValueType y1, ValueType x2, ValueType y2) : X1(x1), Y1(y1), X2(x2), Y2(y2) { }

		bool Valid() const											{ return X1 < X2 && Y1 < Y2; }

		Size GetSize() const										{ return Size(GetWidth(), GetHeight()); }

		ValueType GetWidth() const									{ return X2 - X1; }
		ValueType GetHeight() const									{ return Y2 - Y1; }

		BasicPosition<ValueType> GetTopLeft() const					{ return BasicPosition<ValueType>(X1, Y1); }
		BasicPosition<ValueType> GetRightBottom() const				{ return BasicPosition<ValueType>(X2, Y2); }

		bool operator < (const BasicRect& other) const
		{ return CompareMembersLess(&BasicRect::X1, &BasicRect::Y1, &BasicRect::X2, &BasicRect::Y2)(*this, other); }
		STINGRAYKIT_GENERATE_COMPARISON_OPERATORS_FROM_LESS(BasicRect);

		BasicRect Move(const BasicPosition<ValueType>& pos) const	{ return BasicRect(X1 + pos.X, Y1 + pos.Y, X2 + pos.X, Y2 + pos.Y); }
		BasicRect Move(ValueType dx, ValueType dy) const			{ return BasicRect(X1 + dx, Y1 + dy, X2 + dx, Y2 + dy); }

		BasicRect Intersect(const BasicRect& other) const
		{ return BasicRect(std::max(X1, other.X1), std::max(Y1, other.Y1), std::min(X2, other.X2), std::min(Y2, other.Y2)); }

		bool Intersects(const BasicRect& other) const
		{ return X1 <= other.X2 && X2 >= other.X1 && Y1 <= other.Y2 && Y2 >= other.Y1; }

		std::string ToString() const								{ return StringBuilder() % "[" % X1 % ", " % Y1 % ", "  % X2 % ", " % Y2 % "]"; }

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
	using Rect = BasicRect<int>;

}

#endif
