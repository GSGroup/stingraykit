#ifndef STINGRAYKIT_COLLECTION_FUNCENUMERATOR_H
#define STINGRAYKIT_COLLECTION_FUNCENUMERATOR_H

// Copyright (c) 2011 - 2024, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/collection/IEnumerator.h>
#include <stingraykit/function/function.h>

namespace stingray
{

	/**
	 * @addtogroup toolkit_collections
	 * @{
	 */

	template < typename T >
	class FuncEnumerator : public virtual IEnumerator<T>
	{
		typedef function< T(size_t) >		GetFuncType;
		typedef function< bool(size_t) >	ValidFuncType;

	private:
		GetFuncType			_get;
		ValidFuncType		_valid;
		size_t				_index;

	public:
		FuncEnumerator(const GetFuncType& get, const ValidFuncType& valid)
			: _get(get), _valid(valid), _index(0)
		{ }

		virtual bool Valid() const	{ return _valid(_index); }
		virtual T Get() const		{ return _get(_index); }
		virtual void Next()
		{
			if (!_valid(_index))
				return;
			++_index;
		}
	};

	/** @} */

}

#endif
