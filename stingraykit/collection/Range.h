#ifndef STINGRAYKIT_COLLECTION_RANGE_H
#define STINGRAYKIT_COLLECTION_RANGE_H

#include <utility>
#include <stingraykit/function/function.h>

namespace stingray
{

	template<typename ValueType>
	struct Range
	{
		ValueType Begin;
		ValueType End;

		Range(): Begin(), End() { }

		Range(const ValueType & begin, const ValueType & end): Begin(begin), End(end) { }

		template<typename OutputIterator>
		OutputIterator CopyTo(const OutputIterator &out) const
		{ return std::copy(Begin, End, out); }

		template<typename OutputIterator, typename TransformFunc>
		OutputIterator CopyTo(const TransformFunc & transform, OutputIterator out) const
		{
			for(ValueType i = Begin; i != End; ++i)
				*out++ = transform(*i);
			return out;
		}
	};

	template<typename RangeValueType>
	Range<RangeValueType> MakeRange(const RangeValueType & begin, const RangeValueType & end)
	{ return Range<RangeValueType>(begin, end); }

	template<typename ValueType>
	Range<ValueType> ToRange(const std::pair<ValueType, ValueType> &pair)
	{ return MakeRange(pair.first, pair.second); }

}

#endif
