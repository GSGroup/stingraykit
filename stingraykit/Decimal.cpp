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

		string_view RemoveUnsignificantZeros(string_view source)
		{
			string_view::const_reverse_iterator it = source.rbegin();
			while (it != source.rend() && *it == '0')
				++it;
			return source.substr(0, std::distance(it, source.rend()));
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

	Decimal::Decimal(string_view str)
	{
		STINGRAYKIT_CHECK(!str.empty(), ArgumentException("str"));

		const size_t pointPos = str.find('.');
		if (pointPos == string_view::npos)
		{
			_mantissa = stingray::FromString<s64>(str);
			_exponent = 0;
			return;
		}

		const string_view rawIntegralPart = str.substr(0, pointPos);
		const string_view rawFractionalPart = str.substr(pointPos + 1);
		STINGRAYKIT_CHECK(!rawIntegralPart.empty() || !rawFractionalPart.empty(), ArgumentException("str", str));

		const s64 integralPart = rawIntegralPart.empty() ? 0 : stingray::FromString<s64>(rawIntegralPart);

		const string_view croppedFractionalPart = RemoveUnsignificantZeros(rawFractionalPart);
		const s64 fractionalPart = croppedFractionalPart.empty() ? 0 : stingray::FromString<s64>(croppedFractionalPart);

		const u16 exponent = (u16)croppedFractionalPart.size();
		const s64 integralMantissa = integralPart * std::pow(BaseOfExponent, exponent);
		const s64 fractionalMantissa = integralPart >= 0 ? fractionalPart : -fractionalPart;
		const int signFactor = !rawIntegralPart.empty() && rawIntegralPart[0] == '-' && integralPart == 0 ? -1 : 1;

		_mantissa = (integralMantissa + fractionalMantissa) * signFactor;
		_exponent = exponent;
	}


	std::string Decimal::ToString() const
	{
		const std::string value = AddSignificantZeros(stingray::ToString(_mantissa), _exponent);

		const string_view integralPart = ExtractPrefix(value, value.size() - _exponent);

		const string_view rawFractionalPart = RemovePrefix(value, integralPart);
		const string_view croppedFractionalPart = RemoveUnsignificantZeros(rawFractionalPart);

		StringBuilder builder;
		builder % integralPart;
		if (integralPart.empty() || integralPart == "-")
			builder % "0";

		if (!croppedFractionalPart.empty())
			builder % "." % croppedFractionalPart;
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
