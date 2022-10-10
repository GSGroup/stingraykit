#ifndef STINGRAYKIT_STRING_UTF8ITERATORRANGE_H
#define STINGRAYKIT_STRING_UTF8ITERATORRANGE_H

// Copyright (c) 2011 - 2022, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/collection/RangeBase.h>
#include <stingraykit/exception.h>

namespace stingray
{

	STINGRAYKIT_DECLARE_SIMPLE_EXCEPTION(MalformedUtf8Exception, "Malformed Utf-8 sequence");

	template < typename ContainerType >
	class Utf8IteratorRange : public Range::RangeBase<Utf8IteratorRange<ContainerType>, u32, std::bidirectional_iterator_tag>
	{
	private:
		typedef Utf8IteratorRange<ContainerType>		Self;
		typedef typename ContainerType::const_iterator	IteratorType;

		IteratorType _begin, _end, _it;

	public:
		Utf8IteratorRange(const ContainerType& data) : _begin(data.begin()), _end(data.end()), _it(_begin)
		{ }

		Utf8IteratorRange(const ContainerType& data, const IteratorType& it) : _begin(data.begin()), _end(data.end()), _it(it)
		{ }

		u32 Get() const
		{
			STINGRAYKIT_CHECK(_it != _end, "Get() behind last element");

			IteratorType it = _it;
			u8 c0 = (u8)*it++;
			if (c0 <= 0x7f)
				return c0;

			STINGRAYKIT_CHECK(c0 != 0xc0 && c0 != 0xc1 && c0 < 0xf5 && it != _end, MalformedUtf8Exception());

			u8 c1 = (u8)*it++;
			if (c0 >= 0xc2 && c0 <= 0xdf)
				return ((c0 & 0x1f) << 6) | (c1 & 0x3f);

			STINGRAYKIT_CHECK(it != _end, MalformedUtf8Exception());

			u8 c2 = (u8)*it++;
			if (c0 >= 0xe0 && c0 <= 0xef)
				return ((c0 & 0x0f) << 12) | ((c1 & 0x3f) << 6) | (c2 & 0x3f);

			STINGRAYKIT_CHECK(it != _end, MalformedUtf8Exception());

			u8 c3 = (u8)*it++;
			if (c0 >= 0xf0 && c0 <= 0xf4)
				return ((c0 & 0x07) << 18) | ((c1 & 0x3f) << 12) | ((c2 & 0x3f) << 6) | (c3 & 0x3f);

			STINGRAYKIT_THROW(MalformedUtf8Exception());
		}

		IteratorType GetIterator() const
		{ return _it; }

		bool Valid() const
		{ return _it != _end; }

		bool Equals(const Self& other) const
		{ return _begin == other._begin && _end == other._end && _it == other._it; }

		Self& First()
		{ _it = _begin; return *this; }

		Self& Last()
		{ _it = _end; if (_it != _begin) --_it; return *this; }

		Self& Next()
		{
			STINGRAYKIT_CHECK(_it != _end, "Next() behind last element");
			++_it;
			while ( (_it != _end) && (((u8)(*_it) & 0xc0) == 0x80) )
				++_it;
			return *this;
		}

		Self& Prev()
		{
			STINGRAYKIT_CHECK(_it != _begin, "Prev() at first element");
			--_it;
			while ( (_it != _begin) && (((u8)(*_it) & 0xc0) == 0x80) )
				--_it;
			return *this;
		}
	};

}

#endif
