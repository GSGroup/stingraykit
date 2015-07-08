#ifndef STINGRAYKIT_COLLECTION_TORANGE_H
#define STINGRAYKIT_COLLECTION_TORANGE_H

namespace stingray
{

	namespace Detail
	{
		template <typename T, typename Enabler = void>
		struct ToRangeImpl;
	}


	template <typename T>
	typename Detail::ToRangeImpl<T>::ValueT ToRange(T& src)
	{ return Detail::ToRangeImpl<T>::Do(src); }


	template <typename T>
	typename Detail::ToRangeImpl<const T>::ValueT ToRange(const T& src)
	{ return Detail::ToRangeImpl<const T>::Do(src); }
}

#endif
