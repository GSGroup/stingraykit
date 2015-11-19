#ifndef STINGRAYKIT_COMPRESSION_ARITHMETICDECODER_H
#define STINGRAYKIT_COMPRESSION_ARITHMETICDECODER_H

#include <stingraykit/exception.h>

namespace stingray
{

	class ArithmeticDecoder
	{
	private:
		u32        _low;
		u32        _high;
		u32        _code;

	public:
		template <typename GetBitFunctor_>
		ArithmeticDecoder(const GetBitFunctor_& f) :
			_low(0), _high(0x7FFFFFFF), _code(0)
		{
			for (int i = 0; i < 31; ++i)
				_code = (_code << 1) | f();
		}

		u32 GetProbability(u32 scale) const
		{
			u32 range = (_high - _low + 1) / scale;
			return (_code - _low) / range;
		}

		template <typename GetBitFunctor_>
		void SymbolDecoded(u32 symbolLow, u32 symbolHigh, u32 scale, const GetBitFunctor_& f)
		{
			STINGRAYKIT_CHECK(symbolLow < symbolHigh && symbolHigh <= scale, ArgumentException(StringBuilder() % symbolLow % ", " % symbolHigh % ", " % scale));
			u32 range = (_high - _low + 1) / scale;
			_high = _low + range * symbolHigh - 1;
			_low = _low + range * symbolLow;
			while (true)
			{
				if (FirstBit(_low) == FirstBit(_high))
					Shift(f);
				else if (SecondBit(_low) && !SecondBit(_high))
				{
					_low &= 0x1FFFFFFF;
					_high |= 0x20000000;
					_code = _code ^ 0x20000000;
					Shift(f);
				}
				else
					return;
			}
		}

	private:
		template <typename GetBitFunctor_>
		void Shift(const GetBitFunctor_& f)
		{
			_low = (_low << 1) & 0x7FFFFFFF;
			_high = ((_high << 1) & 0x7FFFFFFF) | 1;
			_code = ((_code << 1) & 0x7FFFFFFF) | f();
		}

		static bool FirstBit(u32 val)  { return (val & 0x40000000) != 0; }
		static bool SecondBit(u32 val) { return (val & 0x20000000) != 0; }
	};

}

#endif
