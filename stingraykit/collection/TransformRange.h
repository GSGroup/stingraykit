#ifndef STINGRAYKIT_COLLECTION_TRANSFORMRANGE_H
#define STINGRAYKIT_COLLECTION_TRANSFORMRANGE_H

#include <stingraykit/function/function.h>
#include <stingraykit/function/function_info.h>
#include <stingraykit/collection/iterators.h>
#include <stingraykit/collection/Range.h>

namespace stingray
{

	template<typename RangeType, typename TransformFunc>
	Range<TransformerIterator<typename RangeType::const_iterator, TransformFunc, EmptyType> > TransformRange(const RangeType& range, const TransformFunc& transformFunc)
	{
		return Range<TransformerIterator<typename RangeType::const_iterator, TransformFunc, EmptyType> >(
			TransformIterator(range.begin(), transformFunc),
			TransformIterator(range.end(), transformFunc));
	}

}

#endif
