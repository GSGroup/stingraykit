// Copyright (c) 2011 - 2024, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.


#include <stingraykit/Decimal.h>
#include <stingraykit/string/StringUtils.h>

#include <cmath>

namespace stingray
{

	namespace
	{

		const u16 BaseOfExponent = 10;

		std::string RemoveUnsignificantZeros(const std::string& source)
		{
			std::string::const_reverse_iterator it = source.rbegin();
			while (it != source.rend() && *it == '0')
				++it;
			return std::string(source.begin(), source.begin() + (source.rend() - it));
		}

		std::string AddSignificantZeros(const std::string& source, u16 exponent)
		{
			if (source.size() < exponent)
				return std::string(exponent - source.size(), '0') + source;
			return source;
		}

		s64 PowToExponent(const Decimal& decimal, u16 exponent)
		{ return decimal.GetMantissa() * std::pow(BaseOfExponent, exponent - decimal.GetExponent()); }

	}

	Decimal::Decimal(const std::string& str)
	{
		std::vector<std::string> fractions;
		Copy(Split(str, ".", 2), std::back_inserter(fractions));
		STINGRAYKIT_CHECK_RANGE(fractions.size(), 1, 3);

		const s64 integralPart = stingray::FromString<s64>(fractions[0]);

		if (fractions.size() == 1)
		{
			_mantissa = integralPart;
			_exponent = 0;
			return;
		}

		const std::string rawFractionalPart = RemoveUnsignificantZeros(fractions[1]);
		const s64 fractionalPart = stingray::FromString<s64>(rawFractionalPart);

		const u16 exponent = (u16)rawFractionalPart.size();
		const s64 integralMantissa = integralPart * std::pow(BaseOfExponent, exponent);
		const s64 fractionalMantissa = (integralPart >= 0) ? fractionalPart : -fractionalPart;
		const int signFactor = (!fractions[0].empty() && fractions[0][0] == '-' && integralPart == 0) ? -1 : 1;

		_mantissa = (integralMantissa + fractionalMantissa) * signFactor;
		_exponent = exponent;
	}


	std::string Decimal::ToString() const
	{
		const std::string value = AddSignificantZeros(stingray::ToString(_mantissa), _exponent);

		std::string integralPart =  ExtractPrefix(value, value.size() - _exponent);
		std::string fractionalPart  = RemoveUnsignificantZeros(RemovePrefix(value, integralPart));

		if (integralPart.empty() || integralPart == "-")
			integralPart.append("0");

		StringBuilder builder;
		builder % integralPart;

		if (!fractionalPart.empty())
			builder % "." % fractionalPart;
		return builder;
	}


	bool Decimal::operator == (const Decimal& other) const
	{
		const u16 maxExponent = std::max(_exponent, other._exponent);
		return PowToExponent(*this, maxExponent) == PowToExponent(other, maxExponent);
	}


	bool Decimal::operator < (const Decimal& other) const
	{
		const u16 maxExponent = std::max(_exponent, other._exponent);
		return PowToExponent(*this, maxExponent) < PowToExponent(other, maxExponent);
	}


	Decimal& Decimal::operator += (const Decimal& other)
	{
		const u16 maxExponent = std::max(_exponent, other._exponent);
		_mantissa = PowToExponent(*this, maxExponent) + PowToExponent(other, maxExponent);
		_exponent = maxExponent;
		return *this;
	}


	Decimal& Decimal::operator *= (const Decimal& other)
	{
		const u16 maxExponent = std::max(_exponent, other._exponent);
		_mantissa = PowToExponent(*this, maxExponent) * PowToExponent(other, maxExponent);
		_exponent = maxExponent + maxExponent;
		return *this;
	}

}
