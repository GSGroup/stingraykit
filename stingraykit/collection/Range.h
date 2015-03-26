#ifndef STINGRAYKIT_COLLECTION_RANGE_H
#define STINGRAYKIT_COLLECTION_RANGE_H

#include <stingraykit/function/function.h>
#include <utility>
#include <algorithm>

namespace stingray
{

	template<typename ValueType>
	struct Range
	{
		ValueType Begin;
		ValueType End;

		Range(): Begin(), End() { }

		Range(const ValueType & begin, const ValueType & end): Begin(begin), End(end) { }

		template<typename OutputIteratorType>
		OutputIteratorType CopyTo(const OutputIteratorType &out) const
		{ return std::copy(Begin, End, out); }

		typedef ValueType		const_iterator;

		ValueType begin() const { return Begin; }
		ValueType end() const	{ return End; }
	};

	template<typename RangeValueType>
	Range<RangeValueType> MakeRange(const RangeValueType & begin, const RangeValueType & end)
	{ return Range<RangeValueType>(begin, end); }

	template<typename ValueType>
	Range<ValueType> ToRange(const std::pair<ValueType, ValueType> &pair)
	{ return MakeRange(pair.first, pair.second); }

	template<typename CollectionType>
	Range<typename CollectionType::const_iterator> ToRange(const CollectionType &collection)
	{ return MakeRange(collection.begin(), collection.end()); }

}

#endif
