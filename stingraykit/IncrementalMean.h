#ifndef STINGRAYKIT_INCREMENTALMEAN_H
#define STINGRAYKIT_INCREMENTALMEAN_H

// Copyright (c) 2011 - 2025, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/Exception.h>

namespace stingray
{

	template < typename T >
	class IncrementalMean
	{
		static_assert(std::is_integral<T>::value && std::is_signed<T>::value, "T must be signed integer");

	private:
		T		_mean;
		T		_remainder;
		T		_count;

	public:
		IncrementalMean() : _mean(0), _remainder(0), _count(0) { }

		void Add(T value)
		{
			STINGRAYKIT_CHECK(_count < std::numeric_limits<T>::max(), "Too many values");

			++_count;

			const T delta = value - _mean + _remainder;
			_mean = _mean + delta / _count;
			_remainder = delta % _count;

			if (_remainder < 0)
			{
				--_mean;
				_remainder += _count;
			}
		}

		T Mean() const { return _mean; }
		T Remainder() const { return _remainder; }
		T Count() const { return _count; }

		void Reset()
		{
			_mean = 0;
			_remainder = 0;
			_count = 0;
		}
	};

}

#endif
