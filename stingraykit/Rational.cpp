// Copyright (c) 2011 - 2025, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/Rational.h>

#include <stingraykit/string/ToString.h>

namespace stingray
{

	namespace
	{

		constexpr Rational::ValueType SafePow10(size_t power)
		{
			constexpr Rational::ValueType MaxMultiplicand = std::numeric_limits<Rational::ValueType>::max() / 10;

			Rational::ValueType res = 1;
			for (size_t i = 0; i < power; ++i)
			{
				STINGRAYKIT_CHECK(res <= MaxMultiplicand, IndexOutOfRangeException((StringBuilder() % "10^" % power).ToString(), std::numeric_limits<Rational::ValueType>::min(), std::numeric_limits<Rational::ValueType>::max()));
				res *= 10;
			}
			return res;
		}

	}


	std::string Rational::ToString() const
	{ return _num == 0 || _denum  == 1 ? stingray::ToString(_num) : StringBuilder() % _num % "/" % _denum; }


	Rational Rational::FromString(string_view str)
	{
		const string_view::size_type slashPos = str.find('/');
		if (slashPos != string_view::npos)
				return Rational(
						stingray::FromString<ValueType>(str.substr(0, slashPos)),
						stingray::FromString<ValueType>(str.substr(slashPos + 1)));

		const string_view::size_type dotPos = str.find('.');

		string_view decimalPart;
		string_view fractionalPart;

		if (dotPos == string_view::npos)
			decimalPart = str;
		else
		{
			decimalPart = str.substr(0, dotPos);
			fractionalPart = str.substr(dotPos + 1);
		}

		STINGRAYKIT_CHECK(!decimalPart.empty() || !fractionalPart.empty(), FormatException(str));
		STINGRAYKIT_CHECK(fractionalPart.empty() || (fractionalPart.front() >= '0' && fractionalPart.front() <= '9'), FormatException(str));

		const ValueType denum = SafePow10(fractionalPart.size());

		ValueType numDecimalPart = decimalPart.empty() ? 0 : stingray::FromString<ValueType>(decimalPart);
		STINGRAYKIT_CHECK(
				numDecimalPart >= 0 ? numDecimalPart <= std::numeric_limits<ValueType>::max() / denum : numDecimalPart >= std::numeric_limits<ValueType>::min() / denum,
				IndexOutOfRangeException((StringBuilder() % numDecimalPart % " * " % denum).ToString(), std::numeric_limits<Rational::ValueType>::min(), std::numeric_limits<Rational::ValueType>::max()));
		numDecimalPart *= denum;

		ValueType numFractionalPart = fractionalPart.empty() ? 0 : stingray::FromString<ValueType>(fractionalPart);
		numFractionalPart = decimalPart.empty() || decimalPart.front() != '-' ? numFractionalPart : -numFractionalPart;
		STINGRAYKIT_CHECK_INTEGER_OVERFLOW(numDecimalPart, numFractionalPart);

		return Rational(numDecimalPart + numFractionalPart, denum);
	}

}
