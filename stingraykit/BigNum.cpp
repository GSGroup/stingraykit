// Copyright (c) 2011 - 2024, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/BigNum.h>

#include <stingraykit/string/ToString.h>

namespace stingray
{

	namespace
	{

		template <typename T>
		size_t BitLength(T x)
		{
			size_t length = 0;
			while (x > 0)
			{
				x >>= 1;
				length++;
			}
			return length;
		}


		size_t CeilingDiv(const size_t& dividend, const size_t& devisor)
		{
			return (dividend + devisor - 1) / devisor;
		}

	}


	std::string BigUnsignedInteger::ToString(size_t base) const
	{
		StringBuilder result;
		ConstByteArray data = Dump(base);
		for (ByteArray::const_iterator it = data.begin(); it != data.end(); ++it)
			result % (*it < 10 ? '0' + *it : 'A' + *it - 10);
		if (result.empty())
			return "0";
		return result;
	}


	void BigUnsignedInteger::ZapLeadingZeroes()
	{
		while (!_units.empty() && _units.back() == 0)
			_units.pop_back();
	}


	void BigUnsignedInteger::LoadFromString(const std::string& source)
	{
		if (source.substr(0, 2) == "0x")
			LoadFromString(source.substr(2), 16);
		else if (source.substr(0, 2) == "0b")
			LoadFromString(source.substr(2), 2);
		else
			LoadFromString(source, 10);
	}


	void BigUnsignedInteger::LoadFromString(const std::string& source, size_t base_)
	{
		_units.clear();

		BigUnsignedInteger base(base_), temp;
		for (size_t i = 0; i < source.length(); ++i)
		{
			u8 value;
			char sym = source[i];
			if (sym >= '0' && sym <= '9')
				value = sym - '0';
			else if (sym >= 'A' && sym <= 'Z')
				value = sym - 'A' + 10;
			else if (sym >= 'a' && sym <= 'z')
				value = sym - 'a' + 10;
			else
				STINGRAYKIT_THROW(ArgumentException("source", source));
			STINGRAYKIT_CHECK(value < base_, ArgumentException("base", base_));

			temp.Multiply(*this, base);
			Add(temp, BigUnsignedInteger(value));
		}
	}


	ByteArray BigUnsignedInteger::Dump(size_t base_) const
	{
		size_t maxBitLength = _units.size() * sizeof(BlockType) * 8;
		size_t minBitsPerDigit = BitLength(base_) - 1;
		size_t maxDigetsLength = CeilingDiv(maxBitLength, minBitsPerDigit);

		std::vector<u8> result;
		result.reserve(maxDigetsLength);
		BigUnsignedInteger x(*this), base(base_);
		while(!x.IsZero())
		{
			BigUnsignedInteger lastDigit(x);
			lastDigit.DivideWithRemainder(base, x);
			result.push_back(lastDigit.ToPrimitive<u8>());
		}

		return ByteArray(result.rbegin(), result.rend());
	}


	BigUnsignedInteger::BlockType BigUnsignedInteger::GetShiftedBlock(const BigUnsignedInteger& num, size_t blockIndex, size_t bits) const
	{
		BlockType part1 = (blockIndex == 0 || bits == 0) ? 0 : (num._units.at(blockIndex - 1) >> (sizeof(BlockType) * 8 - bits));
		BlockType part2 = (blockIndex == num._units.size()) ? 0 : (num._units.at(blockIndex) << bits);
		return part1 | part2;
	}


	void BigUnsignedInteger::Add(const BigUnsignedInteger& summand1, const BigUnsignedInteger& summand2)
	{
		if (&summand1 == this || &summand2 == this)
		{
			BigUnsignedInteger temp;
			temp.Add(summand1, summand2);
			*this = temp;
			return;
		}

		if (summand1.IsZero())
		{
			*this = summand2;
			return;
		}
		if (summand2.IsZero())
		{
			*this = summand1;
			return;
		}

		const std::vector<BlockType>& shorterSummand = summand1._units.size() < summand2._units.size() ? summand1._units : summand2._units;
		const std::vector<BlockType>& longerSummand  = summand1._units.size() < summand2._units.size() ? summand2._units : summand1._units;

		_units.resize(longerSummand.size() + 1);

		size_t index;
		bool carryIn;

		for (index = 0, carryIn = false; index < shorterSummand.size(); ++index)
		{
			BlockType temp = longerSummand.at(index) + shorterSummand.at(index);
			bool carryOut = (temp < longerSummand.at(index));
			if (carryIn)
			{
				++temp;
				carryOut = carryOut || (temp == 0);
			}
			_units[index] = temp;
			carryIn = carryOut;
		}

		for (; index < longerSummand.size() && carryIn; ++index)
		{
			BlockType temp = longerSummand.at(index) + 1;
			carryIn = (temp == 0);
			_units[index] = temp;
		}

		for (; index < longerSummand.size(); ++index)
			_units[index] = longerSummand.at(index);

		if (carryIn)
			_units[index] = 1;
		else
			_units.pop_back();
	}


	void BigUnsignedInteger::Subtract(const BigUnsignedInteger& minuend, const BigUnsignedInteger& subtrahend)
	{
		if (&minuend == this || &subtrahend == this)
		{
			BigUnsignedInteger temp;
			temp.Subtract(minuend, subtrahend);
			*this = temp;
			return;
		}

		if (subtrahend.IsZero())
		{
			*this = minuend;
			return;
		}

		STINGRAYKIT_CHECK(subtrahend._units.size() <= minuend._units.size(), "subtrahend too large");

		_units.resize(minuend._units.size());

		size_t index;
		bool borrowIn;

		for (index = 0, borrowIn = false; index < subtrahend._units.size(); ++index)
		{
			BlockType temp = minuend._units[index] - subtrahend._units[index];
			bool borrowOut = (temp > minuend._units[index]);
			if (borrowIn)
			{
				borrowOut = borrowOut || (temp == 0);
				--temp;
			}
			_units[index] = temp;
			borrowIn = borrowOut;
		}

		for (; index < minuend._units.size() && borrowIn; ++index)
		{
			borrowIn = (minuend._units[index] == 0);
			_units[index] = minuend._units[index] - 1;
		}

		if (borrowIn)
		{
			_units.clear();
			STINGRAYKIT_THROW("Negative result in unsigned calculation");
		}

		for (; index < minuend._units.size(); ++index)
			_units[index] = minuend._units[index];

		ZapLeadingZeroes();
	}


	void BigUnsignedInteger::Multiply(const BigUnsignedInteger& multiplier1, const BigUnsignedInteger& multiplier2)
	{
		if (&multiplier1 == this || &multiplier2 == this)
		{
			BigUnsignedInteger temp;
			temp.Multiply(multiplier1, multiplier2);
			*this = temp;
			return;
		}

		if (multiplier1.IsZero() || multiplier2.IsZero())
		{
			_units.clear();
			return;
		}

		/*
		 * Overall method:
		 *
		 * Set this = 0.
		 * For each 1-bit of `a' (say the `i2'th bit of block `i'):
		 *    Add `b << (i blocks and i2 bits)' to *this.
		 */

		_units.resize(multiplier1._units.size() + multiplier2._units.size());
		std::fill(_units.begin(), _units.end(), 0);

		bool carryIn;
		for (size_t index = 0; index < multiplier1._units.size(); ++index)
		{
			for (size_t i2 = 0; i2 < sizeof(BlockType) * 8; ++i2)
			{
				if ((multiplier1._units[index] & ((BlockType)1 << i2)) == 0)
					continue;

				/*
				 * Add b to this, shifted left i blocks and i2 bits.
				 * j is the index in b, and k = i + j is the index in this.
				 *
				 * `getShiftedBlock', a short inline function defined above,
				 * is now used for the bit handling.  It replaces the more
				 * complex `bHigh' code, in which each run of the loop dealt
				 * immediately with the low bits and saved the high bits to
				 * be picked up next time.  The last run of the loop used to
				 * leave leftover high bits, which were handled separately.
				 * Instead, this loop runs an additional time with j == b.len.
				 */

				size_t k = index, j;
				for (j = 0, carryIn = false; j <= multiplier2._units.size(); ++j, ++k)
				{
					BlockType temp = _units[k] + GetShiftedBlock(multiplier2, j, i2);
					bool carryOut = (temp < _units[k]);
					if (carryIn)
					{
						++temp;
						carryOut = carryOut || (temp == 0);
					}
					_units[k] = temp;
					carryIn = carryOut;
				}

				while (carryIn)
				{
					++_units[k];
					carryIn = (_units[k] == 0);
					++k;
				}
			}
		}

		ZapLeadingZeroes();
	}


	void BigUnsignedInteger::DivideWithRemainder(const BigUnsignedInteger& divisor, BigUnsignedInteger& quotient)
	{
		/*
		 * DIVISION WITH REMAINDER
		 * This monstrous function mods *this by the given divisor b while storing the
		 * quotient in the given object q; at the end, *this contains the remainder.
		 * The seemingly bizarre pattern of inputs and outputs was chosen so that the
		 * function copies as little as possible (since it is implemented by repeated
		 * subtraction of multiples of b from *this).
		 */

		STINGRAYKIT_CHECK(&quotient != this, "Cannot write quotient and remainder into the same variable");

		if (&divisor == this || &divisor == &quotient)
		{
			BigUnsignedInteger tempDivisor(divisor);
			DivideWithRemainder(tempDivisor, quotient);
			return;
		}

		/*
		 * Knuth's definition of mod (which this function uses) is somewhat
		 * different from the C++ definition of % in case of division by 0.
		 *
		 * We let a / 0 == 0 (it doesn't matter much) and a % 0 == a, no
		 * exceptions thrown.  This allows us to preserve both Knuth's demand
		 * that a mod 0 == a and the useful property that
		 * (a / b) * b + (a % b) == a.
		 */
		if (divisor.IsZero())
		{
			quotient._units.clear();
			return;
		}

		/*
		 * If *this.len < b.len, then *this < b, and we can be sure that b doesn't go into
		 * *this at all.  The quotient is 0 and *this is already the remainder (so leave it alone).
		 */
		if (_units.size() < divisor._units.size())
		{
			quotient._units.clear();
			return;
		}

		/*
		 * Overall method:
		 *
		 * For each appropriate i and i2, decreasing:
		 *    Subtract (b << (i blocks and i2 bits)) from *this, storing the
		 *      result in subtractBuf.
		 *    If the subtraction succeeds with a nonnegative result:
		 *        Turn on bit i2 of block i of the quotient q.
		 *        Copy subtractBuf back into *this.
		 *    Otherwise bit i2 of block i remains off, and *this is unchanged.
		 *
		 * Eventually q will contain the entire quotient, and *this will
		 * be left with the remainder.
		 *
		 * subtractBuf[x] corresponds to blk[x], not blk[x+i], since 2005.01.11.
		 * But on a single iteration, we don't touch the i lowest blocks of blk
		 * (and don't use those of subtractBuf) because these blocks are
		 * unaffected by the subtraction: we are subtracting
		 * (b << (i blocks and i2 bits)), which ends in at least `i' zero
		 * blocks. */

		/*
		 * Make sure we have an extra zero block just past the value.
		 *
		 * When we attempt a subtraction, we might shift `b' so
		 * its first block begins a few bits left of the dividend,
		 * and then we'll try to compare these extra bits with
		 * a nonexistent block to the left of the dividend.  The
		 * extra zero block ensures sensible behavior; we need
		 * an extra block in `subtractBuf' for exactly the same reason.
		 */

		size_t originalLength = _units.size();
		_units.push_back(0);

		std::vector<BlockType> subtractBuffer(_units.size());

		quotient._units.resize(originalLength - divisor._units.size() + 1);
		std::fill(quotient._units.begin(), quotient._units.end(), 0);

		size_t index = quotient._units.size();
		while (index)
		{
			--index;
			quotient._units[index] = 0; // TODO: remove
			size_t i2 = sizeof(BlockType) * 8;
			while (i2)
			{
				--i2;
				/*
				 * Subtract b, shifted left i blocks and i2 bits, from *this,
				 * and store the answer in subtractBuf.  In the for loop, `k == i + j'.
				 *
				 * Compare this to the middle section of `multiply'.  They
				 * are in many ways analogous.  See especially the discussion
				 * of `getShiftedBlock'.
				 */
				bool borrowIn = false;
				size_t j = 0, k = index;

				for (; j <= divisor._units.size(); ++j, ++k)
				{
					BlockType temp = _units[k] - GetShiftedBlock(divisor, j, i2);
					bool borrowOut = (temp > _units[k]);
					if (borrowIn)
					{
						borrowOut = borrowOut || (temp == 0);
						--temp;
					}
					subtractBuffer[k] = temp;
					borrowIn = borrowOut;
				}

				for (; k < originalLength && borrowIn; ++k)
				{
					borrowIn = (_units[k] == 0);
					subtractBuffer[k] = _units[k] - 1;
				}

				/*
				 * If the subtraction was performed successfully (!borrowIn),
				 * set bit i2 in block i of the quotient.
				 *
				 * Then, copy the portion of subtractBuf filled by the subtraction
				 * back to *this.  This portion starts with block i and ends--
				 * where?  Not necessarily at block `i + b.len'!  Well, we
				 * increased k every time we saved a block into subtractBuf, so
				 * the region of subtractBuf we copy is just [i, k).
				 */
				if (!borrowIn)
				{
					quotient._units[index] |= ((BlockType)1 << i2);
					while (k > index)
					{
						--k;
						_units[k] = subtractBuffer[k];
					}
				}
			}
		}

		quotient.ZapLeadingZeroes();
		ZapLeadingZeroes();
	}


	void BigUnsignedInteger::BitAnd(const BigUnsignedInteger& op1, const BigUnsignedInteger& op2)
	{
		if (&op1 == this || &op2 == this)
		{
			BigUnsignedInteger temp;
			temp.BitAnd(op1, op2);
			*this = temp;
			return;
		}

		_units.resize(std::min(op1._units.size(), op2._units.size()));
		for (size_t i = 0; i < _units.size(); ++i)
			_units[i] = op1._units[i] & op2._units[i];
		ZapLeadingZeroes();
	}


	void BigUnsignedInteger::BitOr(const BigUnsignedInteger& op1, const BigUnsignedInteger& op2)
	{
		if (&op1 == this || &op2 == this)
		{
			BigUnsignedInteger temp;
			temp.BitOr(op1, op2);
			*this = temp;
			return;
		}

		_units.resize(std::min(op1._units.size(), op2._units.size()));
		for (size_t i = 0; i < _units.size(); ++i)
			_units[i] = op1._units[i] | op2._units[i];
		if (op1._units.size() > _units.size())
			_units.insert(_units.end(), op1._units.begin() + _units.size(), op1._units.end());
		else if (op2._units.size() > _units.size())
			_units.insert(_units.end(), op2._units.begin() + _units.size(), op2._units.end());
	}


	void BigUnsignedInteger::BitXor(const BigUnsignedInteger& op1, const BigUnsignedInteger& op2)
	{
		if (&op1 == this || &op2 == this)
		{
			BigUnsignedInteger temp;
			temp.BitXor(op1, op2);
			*this = temp;
			return;
		}

		_units.resize(std::min(op1._units.size(), op2._units.size()));
		for (size_t i = 0; i < _units.size(); ++i)
			_units[i] = op1._units[i] ^ op2._units[i];
		if (op1._units.size() > _units.size())
			_units.insert(_units.end(), op1._units.begin() + _units.size(), op1._units.end());
		else if (op2._units.size() > _units.size())
			_units.insert(_units.end(), op2._units.begin() + _units.size(), op2._units.end());
		ZapLeadingZeroes();
	}


	void BigUnsignedInteger::BitShiftLeft(const BigUnsignedInteger& value, int bits)
	{
		if (&value == this)
		{
			BigUnsignedInteger temp;
			temp.BitShiftLeft(value, bits);
			*this = temp;
			return;
		}

		if (bits < 0)
		{
			BitShiftRight(value, -bits);
			return;
		}

		size_t shiftBlocks = bits / (sizeof(BlockType) * 8);
		size_t shiftBits = bits % (sizeof(BlockType) * 8);

		_units.resize(value._units.size() + shiftBlocks + 1);

		for (size_t i = 0; i < shiftBlocks; ++i)
			_units[i] = 0;

		for (size_t i = shiftBlocks, j = 0; j <= value._units.size(); ++i, ++j)
			_units[i] = GetShiftedBlock(value, j, shiftBits);

		ZapLeadingZeroes();
	}


	void BigUnsignedInteger::BitShiftRight(const BigUnsignedInteger& value, int bits)
	{
		if (&value == this)
		{
			BigUnsignedInteger temp;
			temp.BitShiftRight(value, bits);
			*this = temp;
			return;
		}

		if (bits < 0)
		{
			BitShiftLeft(value, -bits);
			return;
		}

		size_t rightShiftBlocks = CeilingDiv(bits, sizeof(BlockType) * 8);
		size_t leftShiftBits = sizeof(BlockType) * 8 * rightShiftBlocks - bits;

		if (rightShiftBlocks >= value._units.size() + 1)
		{
			_units.clear();
			return;
		}

		_units.resize(value._units.size() + 1 - rightShiftBlocks);
		for (size_t i = 0, j = rightShiftBlocks; j <= value._units.size(); ++i, ++j)
			_units[i] = GetShiftedBlock(value, j, leftShiftBits);

		ZapLeadingZeroes();
	}


	bool BigUnsignedInteger::operator <(const BigUnsignedInteger& other) const
	{
		if (_units.size() < other._units.size())
			return true;

		if (_units.size() > other._units.size())
			return false;

		for (size_t i = 0; i < _units.size(); ++i)
			if (_units[i] < other._units[i])
				return true;

		return false;
	}

}
