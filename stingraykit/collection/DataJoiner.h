#ifndef STINGRAYKIT_COLLECTION_DATAJOINER_H
#define STINGRAYKIT_COLLECTION_DATAJOINER_H

// Copyright (c) 2011 - 2019, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/collection/iterators.h>
#include <stingraykit/Tuple.h>
#include <stingraykit/variant.h>

namespace stingray
{

	/**
	 * @addtogroup toolkit_collections
	 * @{
	 */

	namespace Detail
	{
		template<typename Container>
		struct ConstIteratorGetter
		{
			typedef typename Container::const_iterator ValueT;
		};

		template<typename T_, int Index_>
		struct IndexedType
		{
			typedef T_ Type;
			static const int Index = Index_;

			Type Value;

			IndexedType(const Type& val) : Value(val)
			{}
		};

		template<typename TypeList>
		struct TypeListIndexer
		{
			template<size_t Index>
			struct Functor
			{
				typedef IndexedType<typename GetTypeListItem<TypeList, Index>::ValueT, Index> ValueT;
			};

			typedef typename GenerateTypeList<GetTypeListLength<TypeList>::Value, Functor>::ValueT ValueT;
		};
	}

	template<typename ContainersTypeList>
	struct DataJoinerIterator : public iterator_base<DataJoinerIterator<ContainersTypeList>, const u8, std::random_access_iterator_tag>
	{
		typedef DataJoinerIterator<ContainersTypeList> MyType;
		typedef iterator_base<DataJoinerIterator<ContainersTypeList>, const u8, std::random_access_iterator_tag> base;

		typedef typename TypeListTransform<ContainersTypeList, AddConstPointer>::ValueT ConstContainersPtr;

		typedef typename TypeListTransform<ContainersTypeList, Detail::ConstIteratorGetter>::ValueT IteratorsList;
		typedef typename Detail::TypeListIndexer<IteratorsList>::ValueT IndexedIterators;
		typedef variant<IndexedIterators> IteratorVariant;

		static const size_t ContainersCount = GetTypeListLength<ContainersTypeList>::Value;

		typedef typename base::reference		reference;
		typedef typename base::pointer			pointer;
		typedef typename base::difference_type	difference_type;

	private:
		typedef const Tuple<ConstContainersPtr> TupleType;

		const TupleType	*_containers;
		IteratorVariant	_iterator;

	public:
		DataJoinerIterator(const TupleType* containers, const IteratorVariant& current) :
			_containers(containers),
			_iterator(current)
		{ }

		reference dereference() const							{ return *apply_visitor(DereferenceVisitor(), _iterator); }
		bool equal(const MyType& other) const					{ return distance_to(other) == 0; }

		void increment()										{ advance(1); }
		void decrement()										{ advance(-1); }

		void advance(difference_type diff)						{ SetDistanceFromBegin(GetDistanceFromBegin() + diff); }
		difference_type distance_to(const MyType &other) const	{ return other.GetDistanceFromBegin() - GetDistanceFromBegin(); }

		int which() const										{ return _iterator.which(); }

		static DataJoinerIterator CreateBegin(const TupleType* c)
		{
			typedef typename IndexedIterators::ValueT IterType;
			DataJoinerIterator result(c, IterType(c->template Get<0>()->begin()));
			result.SetDistanceFromBegin(0); // first container can be empty
			return result;
		}

		static DataJoinerIterator CreateEnd(const TupleType* c)
		{
			typedef typename GetTypeListItem<IndexedIterators, ContainersCount - 1>::ValueT IterType;
			return DataJoinerIterator(c, IterType(c->template Get<ContainersCount - 1>()->end()));
		}

		template <typename VisitorType>
		static void ApplyToIntervals(DataJoinerIterator from, DataJoinerIterator to, VisitorType& visitor)
		{
			STINGRAYKIT_CHECK(from._containers == to._containers, "Iterators are from different containers!");
			For<ContainersCount, IntervalsFunctor<VisitorType>::template Func>::Do(from, to, wrap_ref(visitor));
		}

	private:
		struct DereferenceVisitor : static_visitor<pointer>
		{
			template<typename Iter>
			pointer operator()(const Iter& iter) const { return &(*iter.Value); }
		};


		template <typename VisitorType>
		struct IntervalsFunctor
		{
			template<size_t ContainerIndex>
			struct Func
			{
				static void Call(DataJoinerIterator from, DataJoinerIterator to, VisitorType& visitor)
				{
					typename GetTypeListItem<typename TupleType::TypeList, ContainerIndex>::ValueT container = from._containers->template Get<ContainerIndex>();
					if (from.which() <= (int)ContainerIndex && (int)ContainerIndex <= to.which())
					{
						typedef typename GetTypeListItem<IndexedIterators, ContainerIndex>::ValueT IndexedIterType;
						typedef typename GetTypeListItem<IteratorsList, ContainerIndex>::ValueT IterType;
						IterType first = from.which() == ContainerIndex ? variant_get<IndexedIterType>(from._iterator).Value : container->begin();
						IterType last = to.which() == ContainerIndex ? variant_get<IndexedIterType>(to._iterator).Value : container->end();
						if (first != last)
							visitor(first, last);
					}
				}
			};
		};


		template<size_t ContainerIndex>
		struct DistanceFunctor
		{
			static bool Call(const TupleType *cont, const IteratorVariant& iter, difference_type& result)
			{
				typename GetTypeListItem<typename TupleType::TypeList, ContainerIndex>::ValueT container = cont->template Get<ContainerIndex>();
				if (iter.which() != ContainerIndex)
				{
					result += container->size();
					return true;
				}
				typedef typename GetTypeListItem<IndexedIterators, ContainerIndex>::ValueT IterType;
				result += std::distance(container->begin(), variant_get<IterType>(iter).Value);
				return false;
			}
		};

		difference_type GetDistanceFromBegin() const
		{
			difference_type result = 0;
			if (ForIf<ContainersCount, DistanceFunctor>::Do(_containers, _iterator, wrap_ref(result)))
				STINGRAYKIT_THROW("Unknown index!");
			return result;
		}

		template<size_t ContainerIndex>
		struct AdvanceFunctor
		{
			static bool Call(const TupleType *cont, size_t& diff, IteratorVariant& iter)
			{
				typename GetTypeListItem<typename TupleType::TypeList, ContainerIndex>::ValueT container = cont->template Get<ContainerIndex>();
				size_t size = container->size();
				if (diff >= size)
				{
					diff -= size;
					return true;
				}

				typedef typename GetTypeListItem<IndexedIterators, ContainerIndex>::ValueT IterType;
				IterType it(next_iterator(container->begin(), diff));
				iter = it;
				return false;
			}
		};

		void SetDistanceFromBegin(size_t offset)
		{
			if (ForIf<ContainersCount, AdvanceFunctor>::Do(_containers, wrap_ref(offset), wrap_ref(_iterator)))
			{
				STINGRAYKIT_CHECK(offset == 0, "Offset is too big!");
				typedef typename GetTypeListItem<IndexedIterators, ContainersCount - 1>::ValueT IterType;
				_iterator = IterType(_containers->template Get<ContainersCount - 1>()->end());
			}
		}
	};


	template<typename ContainersTypeList>
	struct DataJoiner
	{
	private:
		typedef typename TypeListTransform<ContainersTypeList, AddConstPointer>::ValueT ConstContainersPtr;

		static const size_t ContainersCount = GetTypeListLength<ContainersTypeList>::Value;

	public:
		typedef Tuple<ConstContainersPtr>				TupleType;
		typedef DataJoinerIterator<ContainersTypeList>	iterator;
		typedef iterator								const_iterator;

	private:
		Tuple<ConstContainersPtr>	_containers;

	public:
		template<typename Tuple_>
		DataJoiner(const Tuple_& containers) : _containers(TupleConstructorTag(), containers)
		{ }

		const_iterator begin() const	{ return iterator::CreateBegin(&_containers); }
		const_iterator end() const		{ return iterator::CreateEnd(&_containers); }

		size_t size() const
		{
			size_t result = 0;
			For<GetTypeListLength<ContainersTypeList>::Value, SizeFunctor>::Do(&_containers, wrap_ref(result));
			return result;
		}

		template <typename VisitorType>
		static void ApplyToIntervals(const_iterator from, const_iterator to, VisitorType& visitor)
		{ iterator::ApplyToIntervals(from, to, visitor); }

		template <typename VisitorType>
		static void ApplyToIntervals(const_iterator from, const_iterator to, const VisitorType& visitor)
		{ iterator::template ApplyToIntervals<const VisitorType>(from, to, visitor); }

	private:
		template<size_t ContainerIndex>
		struct SizeFunctor
		{
			static void Call(const TupleType *cont, size_t& result)
			{
				typename GetTypeListItem<typename TupleType::TypeList, ContainerIndex>::ValueT container = cont->template Get<ContainerIndex>();
				result += container->size();
			}
		};
	};

	/** @} */

}

#endif
