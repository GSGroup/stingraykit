#ifndef STINGRAYKIT_COLLECTION_REFCOUNTINGVALUE_H
#define STINGRAYKIT_COLLECTION_REFCOUNTINGVALUE_H

// Copyright (c) 2011 - 2017, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/optional.h>

namespace stingray
{

	template < typename Value_ >
	class RefCountingValue
	{
	private:
		size_t				_refs;
		optional<Value_>	_value;

	public:
		RefCountingValue() : _refs() { }

		size_t count() const
		{ return _refs; }

		template < typename DoSetFunc >
		Value_& set(const DoSetFunc& doSetFunc)
		{
			if (_refs++ == 0)
				_value.emplace(doSetFunc());

			return *_value;
		}

		optional<Value_>& get()
		{ return _value; }

		const optional<Value_>& get() const
		{ return _value; }

		template < typename DoReleaseFunc >
		bool release(const DoReleaseFunc& doReleaseFunc)
		{
			if (_refs == 0)
				return false;

			if (--_refs == 0)
			{
				doReleaseFunc(*_value);
				_value.reset();
			}

			return true;
		}

		template < typename DoReleaseFunc >
		void reset(const DoReleaseFunc& doReleaseFunc)
		{
			if (_refs == 0)
				return;

			_refs = 0;

			doReleaseFunc(*_value);
			_value.reset();
		}
	};

}

#endif
