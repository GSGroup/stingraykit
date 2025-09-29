#ifndef STINGRAYKIT_RATIONAL_H
#define STINGRAYKIT_RATIONAL_H

// Copyright (c) 2011 - 2025, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/math.h>

namespace stingray
{

	class Rational
	{
	public:
		using ValueType = int;

	private:
		ValueType			_num;
		ValueType			_denum;

	public:
		constexpr explicit Rational(ValueType num = 0, ValueType denum = 1)
			:	_num(num),
				_denum(denum)
		{
			STINGRAYKIT_CHECK(_denum, ArgumentException("denum"));

			const ValueType gcd = Gcd(Abs(_num), Abs(_denum));

			_num /= gcd;
			_denum /= gcd;

			if (_denum < 0)
			{
				_num = -_num;
				_denum = -_denum;
			}
		}

		constexpr ValueType Num() const			{ return _num; }
		constexpr ValueType Denum() const		{ return _denum; }

		constexpr bool operator < (const Rational& other) const
		{
			ValueType aq = _num / _denum;
			ValueType ar = _num % _denum;
			while (ar < 0)
			{
				--aq;
				ar += _denum;
			}

			ValueType cq = other._num / other._denum;
			ValueType cr = other._num % other._denum;
			while (cr < 0)
			{
				--cq;
				cr += other._denum;
			}

			return ComparePositiveFractionsLess(_denum, aq, ar, other._denum, cq, cr);
		}
		STINGRAYKIT_GENERATE_CONSTEXPR_COMPARISON_OPERATORS_FROM_LESS(Rational);

		std::string ToString() const;
		static Rational FromString(string_view str);

	private:
		constexpr static bool ComparePositiveFractionsLess(ValueType ad, ValueType aq, ValueType ar, ValueType cd, ValueType cq, ValueType cr)
		{
			if (aq != cq)
				return aq < cq;

			if (ar == 0 || cr == 0)
				return ar < cr;

			return ComparePositiveFractionsLess(cr, cd / cr, cd % cr, ar, ad / ar, ad % ar);
		}
	};


	constexpr bool operator == (const Rational& lhs, Rational::ValueType rhs)	{ return lhs.Denum() == 1 && lhs.Num() == rhs; }
	STINGRAYKIT_GENERATE_NON_MEMBER_BY_VALUE_COMMUTATIVE_EQUALITY_OPERATORS_FROM_EQUAL(constexpr, const Rational&, Rational::ValueType);


	constexpr bool operator < (const Rational& lhs, Rational::ValueType rhs)	{ return lhs < Rational(rhs); }
	constexpr bool operator < (Rational::ValueType lhs, const Rational& rhs)	{ return Rational(lhs) < rhs; }
	STINGRAYKIT_GENERATE_NON_MEMBER_BY_VALUE_RELATIONAL_OPERATORS_FROM_LESS(constexpr, const Rational&, Rational::ValueType);
	STINGRAYKIT_GENERATE_NON_MEMBER_BY_VALUE_RELATIONAL_OPERATORS_FROM_LESS(constexpr, Rational::ValueType, const Rational&);

}

#endif	/* RATIONAL_H */
