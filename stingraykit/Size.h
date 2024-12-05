#ifndef STINGRAYKIT_SIZE_H
#define STINGRAYKIT_SIZE_H

// Copyright (c) 2011 - 2024, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/operators.h>

#include <string>

namespace stingray
{

	struct Size
	{
		int		Width;
		int		Height;

		Size() : Width(), Height() { }
		Size(int width, int height) : Width(width), Height(height) { }

		bool Valid() const										{ return Width > 0 && Height > 0; }

		bool operator < (const Size& other) const;
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

		std::string ToString() const;

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

		bool operator < (const SizeScale& other) const;
		STINGRAYKIT_GENERATE_COMPARISON_OPERATORS_FROM_LESS(SizeScale);

		std::string ToString() const;

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

}

#endif
