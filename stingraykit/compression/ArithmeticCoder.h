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
