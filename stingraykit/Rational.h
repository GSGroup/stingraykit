#ifndef STINGRAYKIT_RATIONAL_H
#define STINGRAYKIT_RATIONAL_H

// Copyright (c) 2011 - 2025, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

namespace stingray
{

	class Rational
	{
	public:
		using ValueType = int;

	private:
		ValueType			_num;
		ValueType			_denum;

	public:
		constexpr explicit Rational(ValueType num = 0, ValueType denum = 1)
			:	_num(num),
				_denum(denum)
		{ }

		constexpr ValueType Num() const			{ return _num; }
		constexpr ValueType Denum() const		{ return _denum; }
	};

}

#endif	/* RATIONAL_H */
