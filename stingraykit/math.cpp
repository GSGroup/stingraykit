// Copyright (c) 2011 - 2022, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/math.h>

#include <stingraykit/string/ToString.h>

namespace stingray
{

	namespace
	{

		FractionInfo RoundFraction(u64 fraction, unsigned precision, unsigned targetPrecision, unsigned radix)
		{
			const unsigned overflowThreshold = radix / 2 + radix % 2;

			bool wasOverflow = false;
			for (; precision > targetPrecision; --precision)
			{
				const bool isOverflow = fraction % radix >= overflowThreshold;
				wasOverflow |= isOverflow;

				fraction = fraction / radix + (isOverflow ? 1 : 0);
			}

			if (!wasOverflow)
				return { fraction, precision, false };

			u64 precisionBase = 1;
			for (unsigned index = 0; index < precision; ++index)
				precisionBase *= radix;

			const bool isOverflow = fraction >= precisionBase;
			if (isOverflow)
				fraction -= precisionBase;

			return { fraction, fraction == 0 ? 0 : precision, isOverflow };
		}

	}


	FractionInfo CalculateFractionRemainder(u64 dividend, u64 divisor, unsigned targetPrecision, unsigned radix)
	{
		STINGRAYKIT_CHECK(divisor != 0, ArgumentException("divisor"));
		STINGRAYKIT_CHECK(radix > 1, ArgumentException("radix", radix));

		u64 remainder = dividend % divisor;
		if (remainder == 0)
			return { 0, 0, false };

		const u64 maxMultiplicand = std::numeric_limits<u64>::max() / radix;

		unsigned precision = 0;
		for (; remainder <= maxMultiplicand; ++precision)
			remainder *= radix;

		remainder /= divisor;

		return RoundFraction(remainder, precision, targetPrecision, radix);
	}

}
