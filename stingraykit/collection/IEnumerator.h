#ifndef STINGRAYKIT_COLLECTION_IENUMERATOR_H
#define STINGRAYKIT_COLLECTION_IENUMERATOR_H

// Copyright (c) 2011 - 2018, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/collection/ToEnumerator.h>
#include <stingraykit/shared_ptr.h>

#define STINGRAYKIT_DECLARE_ENUMERATOR(ClassName) \
		typedef stingray::IEnumerator<ClassName>				ClassName##Enumerator; \
		STINGRAYKIT_DECLARE_PTR(ClassName##Enumerator)

namespace stingray
{

	/**
	 * @addtogroup toolkit_collections
	 * @{
	 */

	template < typename T >
	struct IEnumerator
	{
		typedef T		ItemType;

		virtual ~IEnumerator() { }

		virtual bool Valid() const = 0;
		virtual T Get() const = 0;
		virtual void Next() = 0;
	};


	template < typename T >
	struct IsEnumerator : IsInherited1ParamTemplate<T, IEnumerator> { };


	namespace Detail
	{
		template< typename T >
		struct ToEnumeratorImpl<T, typename EnableIf<IsEnumerator<T>::Value, void>::ValueT>
		{
			typedef IEnumerator<typename T::ItemType>	ValueT;

			static shared_ptr<ValueT> Do(const shared_ptr<T>& src) { return src; }
		};
	}


	/** @} */

}


#endif
