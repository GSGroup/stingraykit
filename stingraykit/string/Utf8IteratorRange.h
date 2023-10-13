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
		using Self = Utf8IteratorRange<ContainerType>;
		using IteratorType = typename ContainerType::const_iterator;

	private:
		IteratorType			_begin;
		IteratorType			_end;
		IteratorType			_it;

	public:
		Utf8IteratorRange(const ContainerType& data) : _begin(data.begin()), _end(data.end()), _it(_begin)
		{ }

		Utf8IteratorRange(const ContainerType& data, const IteratorType& it) : _begin(data.begin()), _end(data.end()), _it(it)
		{ }

		u32 Get() const
		{
			STINGRAYKIT_CHECK(_it != _end, "Get() behind last element");

			IteratorType it = _it;
			const u8 c0 = (u8)*it++;
			if (c0 <= 0x7f)
				return c0;

			STINGRAYKIT_CHECK((c0 & 0xc0) != 0x80, MalformedUtf8Exception());
			STINGRAYKIT_CHECK(c0 != 0xc0, MalformedUtf8Exception());
			STINGRAYKIT_CHECK(c0 != 0xc1, MalformedUtf8Exception());
			STINGRAYKIT_CHECK(c0 < 0xf5, MalformedUtf8Exception());
			STINGRAYKIT_CHECK(it != _end, MalformedUtf8Exception());

			const u8 c1 = (u8)*it++;
			STINGRAYKIT_CHECK((c1 & 0xc0) == 0x80, MalformedUtf8Exception());

			if (c0 >= 0xc2 && c0 <= 0xdf)
				return ((u32)(c0 & 0x1f) << 6) | (c1 & 0x3f);

			STINGRAYKIT_CHECK(it != _end, MalformedUtf8Exception());

			const u8 c2 = (u8)*it++;
			STINGRAYKIT_CHECK((c2 & 0xc0) == 0x80, MalformedUtf8Exception());

			if (c0 >= 0xe0 && c0 <= 0xef)
				return ((u32)(c0 & 0x0f) << 12) | ((u32)(c1 & 0x3f) << 6) | (c2 & 0x3f);

			STINGRAYKIT_CHECK(it != _end, MalformedUtf8Exception());

			const u8 c3 = (u8)*it++;
			STINGRAYKIT_CHECK((c3 & 0xc0) == 0x80, MalformedUtf8Exception());

			if (c0 >= 0xf0 && c0 <= 0xf4)
				return ((u32)(c0 & 0x07) << 18) | ((u32)(c1 & 0x3f) << 12) | ((u32)(c2 & 0x3f) << 6) | (c3 & 0x3f);

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
		{
			_it = _end;
			if (_it != _begin)
				--_it;
			return *this;
		}

		Self& Next()
		{
			STINGRAYKIT_CHECK(_it != _end, "Next() behind last element");

			const u8 c0 = (u8)*_it++;
			if (c0 <= 0x7f)
				return *this;

			STINGRAYKIT_CHECK((c0 & 0xc0) != 0x80, MalformedUtf8Exception());
			STINGRAYKIT_CHECK(c0 != 0xc0, MalformedUtf8Exception());
			STINGRAYKIT_CHECK(c0 != 0xc1, MalformedUtf8Exception());
			STINGRAYKIT_CHECK(c0 < 0xf5, MalformedUtf8Exception());
			STINGRAYKIT_CHECK(_it != _end, MalformedUtf8Exception());

			const u8 c1 = (u8)*_it++;
			STINGRAYKIT_CHECK((c1 & 0xc0) == 0x80, MalformedUtf8Exception());

			if (c0 >= 0xc2 && c0 <= 0xdf)
				return *this;

			STINGRAYKIT_CHECK(_it != _end, MalformedUtf8Exception());

			const u8 c2 = (u8)*_it++;
			STINGRAYKIT_CHECK((c2 & 0xc0) == 0x80, MalformedUtf8Exception());

			if (c0 >= 0xe0 && c0 <= 0xef)
				return *this;

			STINGRAYKIT_CHECK(_it != _end, MalformedUtf8Exception());

			const u8 c3 = (u8)*_it++;
			STINGRAYKIT_CHECK((c3 & 0xc0) == 0x80, MalformedUtf8Exception());

			if (c0 >= 0xf0 && c0 <= 0xf4)
				return *this;

			STINGRAYKIT_THROW(MalformedUtf8Exception());
		}

		Self& Prev()
		{
			STINGRAYKIT_CHECK(_it != _begin, "Prev() at first element");
			--_it;

			for (size_t count = 0; count < 4; ++count, --_it)
			{
				const u8 c0 = (u8)*_it;

				if ((c0 & 0xc0) != 0x80)
				{
					STINGRAYKIT_CHECK(c0 != 0xc0, MalformedUtf8Exception());
					STINGRAYKIT_CHECK(c0 != 0xc1, MalformedUtf8Exception());
					STINGRAYKIT_CHECK(c0 < 0xf5, MalformedUtf8Exception());

					switch (count)
					{
					case 0:			STINGRAYKIT_CHECK(c0 <= 0x7f, MalformedUtf8Exception()); break;
					case 1:			STINGRAYKIT_CHECK(c0 >= 0xc2 && c0 <= 0xdf, MalformedUtf8Exception()); break;
					case 2:			STINGRAYKIT_CHECK(c0 >= 0xe0 && c0 <= 0xef, MalformedUtf8Exception()); break;
					case 3:			STINGRAYKIT_CHECK(c0 >= 0xf0 && c0 <= 0xf4, MalformedUtf8Exception()); break;
					default:		STINGRAYKIT_THROW(MalformedUtf8Exception());
					}

					return *this;
				}

				if (_it == _begin)
					break;
			}

			STINGRAYKIT_THROW(MalformedUtf8Exception());
		}
	};

}

#endif
