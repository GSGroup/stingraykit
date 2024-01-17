#ifndef STINGRAYKIT_COLLECTION_REFCOUNTER_H
#define STINGRAYKIT_COLLECTION_REFCOUNTER_H

// Copyright (c) 2011 - 2024, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stddef.h>

namespace stingray
{

	class RefCounter
	{
	private:
		size_t		_refs;

	public:
		RefCounter() : _refs() { }

		size_t count() const
		{ return _refs; }

		template < typename DoSetFunc >
		void set(const DoSetFunc& doSetFunc)
		{
			if (_refs++ == 0)
				doSetFunc();
		}

		template < typename DoReleaseFunc >
		bool release(const DoReleaseFunc& doReleaseFunc)
		{
			if (_refs == 0)
				return false;

			if (--_refs == 0)
				doReleaseFunc();

			return true;
		}

		template < typename DoReleaseFunc >
		void reset(const DoReleaseFunc& doReleaseFunc)
		{
			if (_refs != 0)
				doReleaseFunc();

			_refs = 0;
		}
	};

}

#endif
