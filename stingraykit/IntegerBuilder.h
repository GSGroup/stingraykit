#ifndef STINGRAYKIT_INTEGERBUILDER_H
#define STINGRAYKIT_INTEGERBUILDER_H

// Copyright (c) 2011 - 2019, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.


#include <stingraykit/metaprogramming/TypeTraits.h>


namespace stingray
{

	/**
	 * @addtogroup toolkit_bits
	 * @{
	 */

	template<typename IntegerType>
	class IntegerBuilder
	{
	private:
		IntegerType _result;

	public:
		IntegerBuilder()
			: _result(0)
		{ static_assert(IsFixedWidthInt<IntegerType>::Value, "Integer must have fixed width"); }

		template<typename U>
		IntegerBuilder& Append(const U& field, size_t fieldSize)
		{
			static_assert(IsFixedWidthInt<U>::Value, "Integer must have fixed width");

			_result <<= fieldSize;
			_result |= ResetMostSignificantBits(field, 8 * sizeof(U) - fieldSize);

			return *this;
		}

		IntegerBuilder& Append(bool bit)
		{ return Append(static_cast<u8>(bit), 1); }

		IntegerType GetResult()
		{ return _result; }

	private:
		template<typename U>
		U ResetMostSignificantBits(const U& field, size_t count)
		{ return field & (static_cast<U>(~0) >> count); }
	};

	/** @} */


}

#endif
