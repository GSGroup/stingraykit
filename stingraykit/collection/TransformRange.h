#ifndef STINGRAYKIT_COLLECTION_TRANSFORMRANGE_H
#define STINGRAYKIT_COLLECTION_TRANSFORMRANGE_H

#include <stingraykit/function/function.h>
#include <stingraykit/function/function_info.h>
#include <stingraykit/collection/iterator_base.h>
#include <stingraykit/collection/Range.h>

namespace stingray
{

	namespace Detail
	{
		template<typename FunctionType, typename InputIteratorType>
		class TransformRangeIterator :
			public iterator_base<TransformRangeIterator<FunctionType, InputIteratorType>, typename function_info<FunctionType>::RetType, std::forward_iterator_tag>
		{
			FunctionType			_transform;
			InputIteratorType		_iterator;

			typedef iterator_base<TransformRangeIterator<FunctionType, InputIteratorType>, typename function_info<FunctionType>::RetType, std::forward_iterator_tag> base_type;
			typedef typename base_type::value_type		value_type;

		public:
			TransformRangeIterator(const FunctionType &transform, const InputIteratorType &iterator): _transform(transform), _iterator(iterator) { }

			bool equal(const TransformRangeIterator &other) const
			{ return _iterator == other._iterator; }

			void increment()
			{ ++_iterator; }

			value_type operator*() const
			{ return _transform(*_iterator); }
		};
	}

	template<typename FunctionType, typename InputIteratorType>
	Range<Detail::TransformRangeIterator<FunctionType, InputIteratorType> > TransformRange(const FunctionType & transform, const Range<InputIteratorType> & input)
	{
		typedef Detail::TransformRangeIterator<FunctionType, InputIteratorType>		OutputIteratorType;
		return Range<OutputIteratorType>(OutputIteratorType(transform, input.Begin), OutputIteratorType(transform, input.End));
	}

}

#endif
