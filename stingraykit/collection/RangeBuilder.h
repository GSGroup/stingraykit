#ifndef STINGRAYKIT_COLLECTION_RANGEBUILDER_H
#define STINGRAYKIT_COLLECTION_RANGEBUILDER_H

#include <stingraykit/TypeList.h>
#include <stingraykit/Tuple.h>
#include <stingraykit/collection/Range.h>
#include <stingraykit/string/ToString.h>
#include <stingraykit/string/lexical_cast.h>

namespace stingray
{

	namespace Detail
	{

		template<typename TupleType_>
		class OutputRange
		{
			typedef TupleType_		TupleType;
			TupleType				_tuple;

			template<int Index>
			struct Iterate
			{
				typedef typename GetTypeListItem<typename TupleType::TypeList, Index>::ValueT Type;

				template<typename IteratorType>
				static void Call(const TupleType &tuple, IteratorType *_iter, size_t inputSize)
				{
					IteratorType &iter = *_iter;
					STINGRAYKIT_CHECK(Index < inputSize, "not enough data to fill output range");
					Type ptr = tuple.template Get<Index>();
					typedef typename Depointer<Type>::ValueT ValueType;
					*ptr = lexical_cast<ValueType>(*iter++);
				}
			};


		public:
			OutputRange(const TupleType &tuple): _tuple(tuple) { }

			template<typename IteratorType>
			IteratorType FillFrom(const Range<IteratorType> &inputRange)
			{
				size_t inputSize = inputRange.Distance();
				IteratorType i(inputRange.Begin);
				stingray::For<TupleType::Size, Iterate>::Do(_tuple, &i, inputSize);
				return i;
			}
		};

		template <typename CurrentTypes>
		class OutputRangeBuilder
		{
		protected:
			typedef Tuple<CurrentTypes>		ValueType;
			ValueType						_value;

			typedef Tuple<typename TypeListReverse<CurrentTypes>::ValueT>	ResultType;

		public:
			OutputRangeBuilder(const ValueType &value): _value(value) { }

			template <typename NextType>
			OutputRangeBuilder<typename TypeListPrepend<CurrentTypes, NextType *>::ValueT> operator % (NextType & output)
			{
				typedef typename TypeListPrepend<CurrentTypes, NextType *>::ValueT	NextTypes;
				return OutputRangeBuilder<NextTypes>(Tuple<NextTypes>(&output, _value));
			}

			OutputRange<ResultType> ToRange() const
			{ return OutputRange<ResultType>(ReverseTuple(_value)); }
		};
	}

	struct OutputRangeBuilder : Detail::OutputRangeBuilder<TypeList_0>
	{
		typedef Detail::OutputRangeBuilder<TypeList_0> BaseType;
		OutputRangeBuilder(): BaseType(typename BaseType::ValueType()) { }
	};

}

#endif
