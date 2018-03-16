#ifndef STINGRAYKIT_POINTERTUPLEBUILDER_H
#define STINGRAYKIT_POINTERTUPLEBUILDER_H

// Copyright (c) 2011 - 2017, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

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

		PointerTupleBuilder() : BaseType(BaseType::ValueType())
		{ }
	};

}

#endif
