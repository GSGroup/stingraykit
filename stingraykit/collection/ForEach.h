#ifndef STINGRAYKIT_COLLECTION_FOREACH_H
#define STINGRAYKIT_COLLECTION_FOREACH_H

// Copyright (c) 2011 - 2019, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/collection/EnumerableWrapper.h>

namespace stingray
{

	/**
	 * @addtogroup toolkit_collections
	 * @{
	 *
	 * @brief A foreach loop
	 * @param[in] ... &lt;type&gt; &lt;name&gt; IN &lt;enumerable or enumerator&gt; [ WHERE &lt;some condition&gt; ]
	 * @par Example:
	 * @code
	 * FOR_EACH(ISomeObjectPtr obj IN something->GetObjectCollection())
	 * {
	 *     ISomeOtherObjectPtr other_obj = dynamic_pointer_cast<ISomeOtherObject>(obj);
	 *     if (!other_obj)
	 *         continue;
	 *     if (other_obj->GetProperty() == 1)
	 *         continue;
	 *     other_obj->SomeMethod();
	 * }
	 * // or
	 * FOR_EACH(ISomeOtherObjectPtr other_obj IN something->GetObjectCollection() WHERE other_obj->GetProperty() != 1)
	 *     other_obj->SomeMethod();
	 * @endcode
	 */

#define FOR_EACH(...) FOR_EACH__IMPL(__VA_ARGS__)


	namespace Detail
	{
		template < typename Signature >
		struct GetItemTypeFromItemDecl;

		template < typename T >
		struct GetItemTypeFromItemDecl< void (*)(T) >
		{ typedef T	ValueT; };

		template < typename T >
		bool ForEach_ItemFilter(bool dummy, const T& val) { return static_cast<bool>(val); }
		inline bool ForEach_ItemFilter(bool dummy) { return true; }
	}


#define IN ,
#define WHERE ,
#define FOR_EACH__IMPL(ItemDecl_, SomethingToEnumerate_, ...) \
		for (bool __broken__ = false; !__broken__; __broken__ = true) \
			for (::stingray::shared_ptr<::stingray::IEnumerator<typename ::stingray::Detail::GetItemTypeFromItemDecl<void(*)(ItemDecl_)>::ValueT>> __en__(::stingray::GetEnumeratorCaster(::stingray::ToEnumerator(SomethingToEnumerate_))); \
				 __en__ && __en__->Valid() && !__broken__; \
				 __en__->Next()) \
				 for (bool __dummy_bool__ = true; __dummy_bool__ && !__broken__; ) \
					 for (ItemDecl_ = __en__->Get(); (__dummy_bool__ && ((__dummy_bool__ = false) == false) && ::stingray::Detail::ForEach_ItemFilter(true, ##__VA_ARGS__) && (__broken__ = true)); __broken__ = false)

	/** @} */

}

#endif
