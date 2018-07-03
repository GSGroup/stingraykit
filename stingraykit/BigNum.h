#ifndef STINGRAYKIT_BIGNUM_H
#define STINGRAYKIT_BIGNUM_H

// Copyright (c) 2011 - 2018, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/exception.h>
#include <stingraykit/collection/ByteData.h>
#include <stingraykit/log/Logger.h>
#include <stingraykit/metaprogramming/EnableIf.h>
#include <stingraykit/string/Hex.h>

#include <vector>

namespace stingray
{

	/**
	 * @addtogroup toolkit_math
	 * @{
	 */

	class BigUnsignedInteger
	{
		typedef unsigned long	BlockType;

	private:
		std::vector<BlockType>	_units;

	public:
		BigUnsignedInteger() { }

		explicit BigUnsignedInteger(const std::string& source, size_t base)
		{ LoadFromString(source, base); }

		BigUnsignedInteger(const std::string& source)
		{ LoadFromString(source); }

		BigUnsignedInteger(const char* source)
		{ LoadFromString(source); }

		explicit BigUnsignedInteger(const ConstByteData& source)
		{ LoadFromData(source.begin(), source.end()); }

		template <typename T>
		BigUnsignedInteger(T source, typename EnableIf<sizeof(BlockType) < sizeof(T), Dummy*>::ValueT dummy = 0)
		{
			STINGRAYKIT_CHECK(source >= 0, ArgumentException("source", source));
			std::vector<u8> preloaded;
			while (source)
			{
				preloaded.push_back(source & 0xff);
				source >>= 8;
			}
			LoadFromData(preloaded.rbegin(), preloaded.rend());
		}

		template <typename T>
		BigUnsignedInteger(T source, typename EnableIf<sizeof(BlockType) >= sizeof(T), Dummy*>::ValueT dummy = 0)
		{
			STINGRAYKIT_CHECK(source >= 0, ArgumentException("source", source));
			_units.push_back(source);
		}

		template <typename T>
		T ToPrimitive() const
		{
			if (_units.size() == 0)
				return 0;

			if (_units.size() == 1)
			{
				T result(_units.at(0));
				if ((BlockType)result == _units.at(0))
					return result;
			}
			STINGRAYKIT_THROW(NotImplementedException());
		}

		std::string ToString(size_t base = 16) const;

		ByteArray ToByteArray() const { return Dump(256); }

		BigUnsignedInteger operator +(const BigUnsignedInteger& op) const
		{
			BigUnsignedInteger ans;
			ans.Add(*this, op);
			return ans;
		}

		BigUnsignedInteger operator -(const BigUnsignedInteger& op) const
		{
			BigUnsignedInteger ans;
			ans.Subtract(*this, op);
			return ans;
		}

		BigUnsignedInteger operator *(const BigUnsignedInteger& op) const
		{
			BigUnsignedInteger ans;
			ans.Multiply(*this, op);
			return ans;
		}

		BigUnsignedInteger operator /(const BigUnsignedInteger& op) const
		{
			STINGRAYKIT_CHECK(!op.IsZero(), InvalidOperationException());
			BigUnsignedInteger quotient, remainder;
			remainder = *this;
			remainder.DivideWithRemainder(op, quotient);
			return quotient;
		}

		BigUnsignedInteger operator %(const BigUnsignedInteger& op) const
		{
			STINGRAYKIT_CHECK(!op.IsZero(), InvalidOperationException());
			BigUnsignedInteger quotient, remainder;
			remainder = *this;
			remainder.DivideWithRemainder(op, quotient);
			return remainder;
		}

		BigUnsignedInteger operator &(const BigUnsignedInteger& op) const
		{
			BigUnsignedInteger ans;
			ans.BitAnd(*this, op);
			return ans;
		}

		BigUnsignedInteger operator |(const BigUnsignedInteger& op) const
		{
			BigUnsignedInteger ans;
			ans.BitOr(*this, op);
			return ans;
		}

		BigUnsignedInteger operator ^(const BigUnsignedInteger& op) const
		{
			BigUnsignedInteger ans;
			ans.BitXor(*this, op);
			return ans;
		}

		BigUnsignedInteger operator <<(int bits) const
		{
			BigUnsignedInteger ans;
			ans.BitShiftLeft(*this, bits);
			return ans;
		}

		BigUnsignedInteger operator >>(int bits) const
		{
			BigUnsignedInteger ans;
			ans.BitShiftRight(*this, bits);
			return ans;
		}

		void operator +=(const BigUnsignedInteger& op) { Add(*this, op); }
		void operator -=(const BigUnsignedInteger& op) { Subtract(*this, op); }
		void operator *=(const BigUnsignedInteger& op) { Multiply(*this, op); }

		void operator /=(const BigUnsignedInteger& op)
		{
			STINGRAYKIT_CHECK(!op.IsZero(), InvalidOperationException());
			BigUnsignedInteger quotient;
			DivideWithRemainder(op, quotient);
			*this = quotient;
		}

		void operator %=(const BigUnsignedInteger& op)
		{
			STINGRAYKIT_CHECK(!op.IsZero(), InvalidOperationException());
			BigUnsignedInteger quotient;
			DivideWithRemainder(op, quotient);
		}

		void operator &=(const BigUnsignedInteger& op) { BitAnd(*this, op); }
		void operator |=(const BigUnsignedInteger& op) { BitOr(*this, op); }
		void operator ^=(const BigUnsignedInteger& op) { BitXor(*this, op); }

		void operator <<=(int bits) { BitShiftLeft(*this, bits); }
		void operator >>=(int bits) { BitShiftRight(*this, bits); }

		bool operator <(const BigUnsignedInteger& other) const;
		STINGRAYKIT_GENERATE_COMPARISON_OPERATORS_FROM_LESS(BigUnsignedInteger);

	private:
		bool IsZero() const { return _units.empty(); }

		void ZapLeadingZeroes();

		template <typename InputIteratorT>
		void LoadFromData(const InputIteratorT& begin, const InputIteratorT& end)
		{
			_units.clear();

			BigUnsignedInteger temp;
			for (InputIteratorT it = begin; it != end; ++it)
			{
				temp.Multiply(*this, 256);
				Add(temp, BigUnsignedInteger(*it));
			}
		}

		void LoadFromString(const std::string& source);
		void LoadFromString(const std::string& source, size_t base);

		ByteArray Dump(size_t base) const;

		BlockType GetShiftedBlock(const BigUnsignedInteger& num, size_t blockIndex, size_t bits) const;

		void Add(const BigUnsignedInteger& summand1, const BigUnsignedInteger& summand2);
		void Subtract(const BigUnsignedInteger& minuend, const BigUnsignedInteger& subtrahend);
		void Multiply(const BigUnsignedInteger& multiplier1, const BigUnsignedInteger& multiplier2);

		void DivideWithRemainder(const BigUnsignedInteger& divisor, BigUnsignedInteger& quotient); // put remainder to receiver

		void BitAnd(const BigUnsignedInteger& op1, const BigUnsignedInteger& op2);
		void BitOr(const BigUnsignedInteger& op1, const BigUnsignedInteger& op2);
		void BitXor(const BigUnsignedInteger& op1, const BigUnsignedInteger& op2);

		void BitShiftLeft(const BigUnsignedInteger& a, int bits);
		void BitShiftRight(const BigUnsignedInteger& a, int bits);
	};

	/** @} */

}

#endif
