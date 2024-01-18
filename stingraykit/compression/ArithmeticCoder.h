// Copyright (c) 2011 - 2024, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#ifndef STINGRAYKIT_COMPRESSION_ARITHMETICCODER_H
#define STINGRAYKIT_COMPRESSION_ARITHMETICCODER_H

#include <stingraykit/exception.h>

namespace stingray
{

	class ArithmeticCoder
	{
	private:
		u32 _low;
		u32 _high;
		u32 _underflowBits;

	public:
		ArithmeticCoder() : _low(0), _high(0x7FFFFFFF), _underflowBits(0)
		{ }

		template <typename BitsConsumer_>
		void Encode(u32 symbolLow, u32 symbolHigh, u32 scale, const BitsConsumer_& consumer)
		{
			STINGRAYKIT_CHECK(symbolLow < symbolHigh && symbolHigh <= scale, ArgumentException(StringBuilder() % symbolLow % ", " % symbolHigh % ", " % scale));
			u32 range = (_high - _low + 1) / scale;
			STINGRAYKIT_CHECK(range != 0, StringBuilder() % "Range is zero! State: " % _low % ", " % _high % ", " % scale);
			_high = _low + range * symbolHigh - 1;
			_low = _low + range * symbolLow;
			while (true)
			{
				if (FirstBit(_low) == FirstBit(_high))
				{
					consumer(FirstBit(_low));
					for (u32 i = 0; i < _underflowBits; ++i)
						consumer(!FirstBit(_low));
					_underflowBits = 0;
				}
				else if (SecondBit(_low) && !SecondBit(_high))
				{
					++_underflowBits;
					_low &= 0x1FFFFFFF;
					_high |= 0x20000000;
				}
				else
					return;

				Shift();
			}
		}

		template <typename BitsConsumer_>
		void EndOfData(const BitsConsumer_& consumer)
		{
			while (_low != _high)
			{
				consumer(FirstBit(_low));
				_low = (_low << 1) & 0x7FFFFFFF;
				_high = (_high << 1) & 0x7FFFFFFF;
			}
		}

	private:
		void Shift()
		{
			_low = (_low << 1) & 0x7FFFFFFF;
			_high = ((_high << 1) & 0x7FFFFFFF) | 1;
		}

		static bool FirstBit(u32 val)  { return (val & 0x40000000) != 0; }
		static bool SecondBit(u32 val) { return (val & 0x20000000) != 0; }
	};

}

#endif
