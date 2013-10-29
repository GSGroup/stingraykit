#ifndef STINGRAY_TOOLKIT_DATAJOINER_H
#define STINGRAY_TOOLKIT_DATAJOINER_H


#include <stingray/toolkit/Tuple.h>
#include <stingray/toolkit/variant.h>

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
			template<int Index>
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

		typedef typename TypeListTransform<ContainersTypeList, GetConstPointerType>::ValueT ConstContainersPtr;

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

	private:
		struct DereferenceVisitor : static_visitor<pointer>
		{
			template<typename Iter>
			pointer operator()(const Iter& iter) const { return &(*iter.Value); }
		};

		template<int ContainerIndex>
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
			if (ForIf<ContainersCount, DistanceFunctor>::Do(_containers, _iterator, ref(result)))
				TOOLKIT_THROW("Unknown index!");
			return result;
		}

		template<int ContainerIndex>
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
				IterType it(next(container->begin(), diff));
				iter = it;
				return false;
			}
		};

		void SetDistanceFromBegin(size_t offset)
		{
			if (ForIf<ContainersCount, AdvanceFunctor>::Do(_containers, ref(offset), ref(_iterator)))
			{
				TOOLKIT_CHECK(offset == 0, "Offset is too big!");
				typedef typename GetTypeListItem<IndexedIterators, ContainersCount - 1>::ValueT IterType;
				_iterator = IterType(_containers->template Get<ContainersCount - 1>()->end());
			}
		}
	};


	template<typename ContainersTypeList>
	struct DataJoiner
	{
	private:
		typedef typename TypeListTransform<ContainersTypeList, GetConstPointerType>::ValueT ConstContainersPtr;

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
			For<GetTypeListLength<ContainersTypeList>::Value, SizeFunctor>::Do(&_containers, ref(result));
			return result;
		}

	private:
		template<int ContainerIndex>
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
