#ifndef STINGRAYKIT_DECIMAL_H
#define STINGRAYKIT_DECIMAL_H

// Copyright (c) 2011 - 2024, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/string/string_view.h>
#include <stingraykit/operators.h>
#include <stingraykit/Types.h>

namespace stingray
{

	class Decimal
	{
	private:
		s64			_mantissa;
		u16			_exponent;

	public:
		Decimal(s64 mantissa = 0, u16 exponent = 0)
			:	_mantissa(mantissa),
				_exponent(exponent)
		{ }

		explicit Decimal(string_view str);

		s64 GetMantissa() const
		{ return _mantissa; }

		u16 GetExponent() const
		{ return _exponent; }

		static Decimal FromString(string_view str)
		{ return Decimal(str); }

		std::string ToString() const;

		bool operator == (const Decimal& other) const;
		STINGRAYKIT_GENERATE_EQUALITY_OPERATORS_FROM_EQUAL(Decimal);

		bool operator < (const Decimal& other) const;
		STINGRAYKIT_GENERATE_RELATIONAL_OPERATORS_FROM_LESS(Decimal);

		Decimal& operator += (const Decimal& other);
		Decimal& operator *= (const Decimal& other);

		Decimal operator - () const
		{ return Decimal(-_mantissa, _exponent); }

		Decimal& operator -= (const Decimal& other)
		{ return *this += -other; }

		Decimal operator + (const Decimal& other) const
		{ Decimal res(*this); return res += other; }

		Decimal operator - (const Decimal& other) const
		{ Decimal res(*this); return res -= other; }

		Decimal operator * (const Decimal& other) const
		{ Decimal res(*this); return res *= other; }
	};

}

#endif
