#ifndef STINGRAYKIT_POINTERTUPLEBUILDER_H
#define STINGRAYKIT_POINTERTUPLEBUILDER_H

#include <stingraykit/TypeList.h>
#include <stingraykit/Tuple.h>
#include <stingraykit/string/lexical_cast.h>

namespace stingray
{

	namespace Detail
	{
		template <typename CurrentTypes>
		class PointerTupleBuilder
		{
		protected:
			typedef Tuple<CurrentTypes>		ValueType;
			ValueType						_value;

			typedef Tuple<typename TypeListReverse<CurrentTypes>::ValueT>	ResultType;

		public:
			PointerTupleBuilder(const ValueType &value): _value(value) { }

			template <typename NextType>
			PointerTupleBuilder<typename TypeListPrepend<CurrentTypes, NextType *>::ValueT> operator % (NextType & output)
			{
				typedef typename TypeListPrepend<CurrentTypes, NextType *>::ValueT	NextTypes;
				return PointerTupleBuilder<NextTypes>(Tuple<NextTypes>(&output, _value));
			}

			ResultType Get() const
			{ return ReverseTuple(_value); }
		};
	}


	struct PointerTupleBuilder : Detail::PointerTupleBuilder<TypeList_0>
	{
		typedef Detail::PointerTupleBuilder<TypeList_0> BaseType;

		PointerTupleBuilder() : BaseType(typename BaseType::ValueType())
		{ }
	};

}

#endif
