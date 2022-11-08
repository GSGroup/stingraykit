#ifndef STINGRAYKIT_TUPLE_H
#define STINGRAYKIT_TUPLE_H

// Copyright (c) 2011 - 2022, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/core/Dummy.h>
#include <stingraykit/core/NonCopyable.h>
#include <stingraykit/metaprogramming/EnableIf.h>
#include <stingraykit/metaprogramming/TypeTraits.h>
#include <stingraykit/reference.h>
#include <stingraykit/Types.h>

#include <utility>

namespace stingray
{

	namespace Detail
	{

		template < typename Tuple_, size_t Index >
		struct TupleItemGetter
		{
			static const typename GetTypeListItem<typename Tuple_::Types, Index>::ValueT& Get(const Tuple_& tuple)
			{ return TupleItemGetter<typename Tuple_::Tail, Index - 1>::Get(tuple.GetTail()); }

			static typename GetTypeListItem<typename Tuple_::Types, Index>::ValueT& Get(Tuple_& tuple)
			{ return TupleItemGetter<typename Tuple_::Tail, Index - 1>::Get(tuple.GetTail()); }

			static typename GetTypeListItem<typename Tuple_::Types, Index>::ValueT&& Get(Tuple_&& tuple)
			{ return TupleItemGetter<typename Tuple_::Tail, Index - 1>::Get(std::move(tuple).GetTail()); }
		};

		template < typename Tuple_ >
		struct TupleItemGetter<Tuple_, 0>
		{
			static const typename Tuple_::ValueType& Get(const Tuple_& tuple)
			{ return tuple.GetHead(); }

			static typename Tuple_::ValueType& Get(Tuple_& tuple)
			{ return tuple.GetHead(); }

			static typename Tuple_::ValueType&& Get(Tuple_&& tuple)
			{ return std::move(tuple).GetHead(); }
		};

		template < typename ToTypes, typename FromTypes >
		struct IsConstructibleTypes
		{
			static const bool Value =
					IsConstructible<typename ToTypes::ValueT, typename FromTypes::ValueT>::Value &&
					IsConstructibleTypes<typename ToTypes::Next, typename FromTypes::Next>::Value;
		};

		template < typename ToTypes >
		struct IsConstructibleTypes<ToTypes, TypeListEndNode>
		{ static const bool Value = false; };

		template < typename FromTypes >
		struct IsConstructibleTypes<TypeListEndNode, FromTypes>
		{ static const bool Value = false; };

		template < >
		struct IsConstructibleTypes<TypeListEndNode, TypeListEndNode>
		{ static const bool Value = true; };

	}


	struct TupleConstructorTag { };


	template < typename TypeList_ >
	class Tuple
	{
		STINGRAYKIT_DEFAULTCOPYABLE(Tuple);
		STINGRAYKIT_DEFAULTMOVABLE(Tuple);

	public:
		using Types = TypeList_;
		using Tail = Tuple<typename Types::Next>;
		using ValueType = typename Types::ValueT;

		static const size_t Size = Tail::Size + 1;

	private:
		ValueType	_val;
		Tail		_tail;

	public:
		Tuple() : _val(), _tail() { }

		template < typename T0, typename... Ts, typename EnableIf<
				!IsSame<typename Decay<T0>::ValueT, TupleConstructorTag>::Value &&
				!IsSame<typename Decay<T0>::ValueT, Tuple<Types>>::Value &&
				Detail::IsConstructibleTypes<Types, TypeList<T0, Ts...>>::Value, bool>::ValueT = false >
		Tuple(T0&& p0, Ts&&... args)
			: _val(std::forward<T0>(p0)), _tail(std::forward<Ts>(args)...)
		{ }

		template < typename OtherTypeList_, typename EnableIf<!IsSame<Tuple<OtherTypeList_>, typename Decay<ValueType>::ValueT>::Value && !IsSame<OtherTypeList_, TypeListEndNode>::Value, bool>::ValueT = false >
		Tuple(const Tuple<OtherTypeList_>& other)
			: _val(other.GetHead()), _tail(other.GetTail())
		{ }

		template < typename OtherTypeList_, typename EnableIf<!IsSame<Tuple<OtherTypeList_>, typename Decay<ValueType>::ValueT>::Value && !IsSame<OtherTypeList_, TypeListEndNode>::Value, bool>::ValueT = false >
		Tuple(Tuple<OtherTypeList_>&& other)
			: _val(std::forward<typename OtherTypeList_::ValueT>(other.GetHead())), _tail(std::move(other.GetTail()))
		{ }

		template < typename TupleLikeObject >
		Tuple(const TupleConstructorTag& tag, TupleLikeObject&& tupleLikeObject)
			: _val(std::forward<TupleLikeObject>(tupleLikeObject).template Get<0>()), _tail(tag, std::forward<TupleLikeObject>(tupleLikeObject), integral_constant<size_t, 1>())
		{ }

		template < typename TupleLikeObject, typename IndexOffset >
		Tuple(const TupleConstructorTag& tag, TupleLikeObject&& tupleLikeObject, IndexOffset Dummy)
			: _val(std::forward<TupleLikeObject>(tupleLikeObject).template Get<IndexOffset::Value>()), _tail(tag, std::forward<TupleLikeObject>(tupleLikeObject), integral_constant<size_t, IndexOffset::Value + 1>())
		{ }

		template < typename TupleLikeObject >
		static Tuple CreateFromTupleLikeObject(TupleLikeObject&& tll)
		{ return Tuple(TupleConstructorTag(), std::forward<TupleLikeObject>(tll)); }

		const ValueType& GetHead() const & { return _val; }
		ValueType& GetHead() & { return _val; }
		ValueType&& GetHead() && { return std::forward<ValueType>(_val); }

		const Tail& GetTail() const & { return _tail; }
		Tail& GetTail() & { return _tail; }
		Tail&& GetTail() && { return std::move(_tail); }

		template < size_t Index >
		const typename GetTypeListItem<Types, Index>::ValueT& Get() const &
		{ return Detail::TupleItemGetter<Tuple, Index>::Get(*this); }

		template < size_t Index >
		typename GetTypeListItem<Types, Index>::ValueT& Get() &
		{ return Detail::TupleItemGetter<Tuple, Index>::Get(*this); }

		template < size_t Index >
		typename GetTypeListItem<Types, Index>::ValueT&& Get() &&
		{ return Detail::TupleItemGetter<Tuple, Index>::Get(std::move(*this)); }

		template < typename Type_ >
		const Type_& Get(Dummy dummy = Dummy()) const &
		{ return Detail::TupleItemGetter<Tuple, IndexOfTypeListItem<Types, Type_>::Value>::Get(*this); }

		template < typename Type_ >
		Type_& Get(Dummy dummy = Dummy()) &
		{ return Detail::TupleItemGetter<Tuple, IndexOfTypeListItem<Types, Type_>::Value>::Get(*this); }

		template < typename Type_ >
		Type_&& Get(Dummy dummy = Dummy()) &&
		{ return Detail::TupleItemGetter<Tuple, IndexOfTypeListItem<Types, Type_>::Value>::Get(std::move(*this)); }

		template < typename Type_, size_t Index >
		const Type_& Get(Dummy dummy = Dummy(), Dummy dummy2 = Dummy()) const &
		{ return Detail::TupleItemGetter<Tuple, IndexOfTypeListItem<Types, Type_, Index>::Value>::Get(*this); }

		template < typename Type_, size_t Index >
		Type_& Get(Dummy dummy = Dummy(), Dummy dummy2 = Dummy()) &
		{ return Detail::TupleItemGetter<Tuple, IndexOfTypeListItem<Types, Type_, Index>::Value>::Get(*this); }

		template < typename Type_, size_t Index >
		Type_&& Get(Dummy dummy = Dummy(), Dummy dummy2 = Dummy()) &&
		{ return Detail::TupleItemGetter<Tuple, IndexOfTypeListItem<Types, Type_, Index>::Value>::Get(std::move(*this)); }
	};


	template < >
	class Tuple<TypeListEndNode>
	{
		STINGRAYKIT_DEFAULTCOPYABLE(Tuple<TypeListEndNode>);
		STINGRAYKIT_DEFAULTMOVABLE(Tuple<TypeListEndNode>);

	public:
		using Types = TypeListEndNode;

		static const size_t Size = 0;

		Tuple() { }

		template < typename TupleLikeObject >
		Tuple(const TupleConstructorTag& tag, TupleLikeObject&& tupleLikeObject)
		{ static_assert(GetTypeListLength<typename Decay<TupleLikeObject>::ValueT::Types>::Value == 0, "Tuple like object is too big"); }

		template < typename TupleLikeObject, typename IndexOffset >
		Tuple(const TupleConstructorTag& tag, TupleLikeObject&& tupleLikeObject, IndexOffset Dummy)
		{ static_assert(GetTypeListLength<typename Decay<TupleLikeObject>::ValueT::Types>::Value == IndexOffset::Value, "Tuple like object is too big"); }

		template < typename TupleLikeObject >
		static Tuple CreateFromTupleLikeObject(TupleLikeObject&& tll)
		{ return Tuple(TupleConstructorTag(), std::forward<TupleLikeObject>(tll)); }
	};


	template < typename... Ts >
	Tuple<TypeList<typename UnwrapReferenceDecay<Ts>::ValueT...>> MakeTuple(Ts&&... args)
	{ return Tuple<TypeList<typename UnwrapReferenceDecay<Ts>::ValueT...>>(std::forward<Ts>(args)...); }


	template < typename... Ts >
	Tuple<TypeList<Ts&&...>> ForwardAsTuple(Ts&&... args)
	{ return Tuple<TypeList<Ts&&...>>(std::forward<Ts>(args)...); }


	namespace Detail
	{

		template < typename TupleOfTuples >
		struct GetHeadSize
		{ static const size_t Value = Decay<typename TupleOfTuples::ValueType>::ValueT::Size; };

		template < >
		struct GetHeadSize<Tuple<TypeListEndNode>>
		{ static const size_t Value = 0; };

		template < typename TupleOfTuples, size_t Index, bool = Index >= GetHeadSize<TupleOfTuples>::Value >
		struct TupleOfTuplesIndexes
		{
			using Next = TupleOfTuplesIndexes<typename TupleOfTuples::Tail, Index - GetHeadSize<TupleOfTuples>::Value>;

			static const size_t TupleIndex = Next::TupleIndex + 1;
			static const size_t ValueIndex = Next::ValueIndex;
		};

		template < typename TupleOfTuples, size_t Index >
		struct TupleOfTuplesIndexes<TupleOfTuples, Index, false>
		{
			static const size_t TupleIndex = 0;
			static const size_t ValueIndex = Index;
		};

		template < typename... Tuples >
		class TupleConcater
		{
			template < typename Tuple >
			struct GetTupleTypeList
			{ using ValueT = typename Decay<Tuple>::ValueT::Types; };

		public:
			using Types = typename TypeListMerge<typename TypeListTransform<TypeList<Tuples...>, GetTupleTypeList>::ValueT>::ValueT;

		private:
			using TupleOfTuplesTypes = TypeList<Tuples...>;
			using TupleOfTuples = Tuple<TupleOfTuplesTypes>;

		private:
			TupleOfTuples	_tuples;

		public:
			explicit TupleConcater(Tuples&&... tuples) : _tuples(std::forward<Tuples>(tuples)...) { }

			template < size_t Index, typename Indexes = TupleOfTuplesIndexes<TupleOfTuples, Index> >
			const typename GetTypeListItem<Types, Index>::ValueT& Get() const &
			{ return _tuples.template Get<Indexes::TupleIndex>().template Get<Indexes::ValueIndex>(); }

			template < size_t Index, typename Indexes = TupleOfTuplesIndexes<TupleOfTuples, Index> >
			typename EnableIf<
					IsNonConstRvalueReference<typename GetTypeListItem<TupleOfTuplesTypes, Indexes::TupleIndex>::ValueT&&>::Value,
					typename GetTypeListItem<Types, Index>::ValueT>::ValueT&& Get() &&
			{ return std::move(_tuples).template Get<Indexes::TupleIndex>().template Get<Indexes::ValueIndex>(); }
		};

	}


	template < typename... Tuples >
	auto ConcatTuples(Tuples&&... tuples)
	{
		using TupleConcater = Detail::TupleConcater<Tuples...>;
		return Tuple<typename TupleConcater::Types>::CreateFromTupleLikeObject(TupleConcater(std::forward<Tuples>(tuples)...));
	}


	namespace Detail
	{

		template < typename TupleLikeObject_ >
		class TupleReverser
		{
		public:
			using SrcTypes = typename Decay<TupleLikeObject_>::ValueT::Types;
			using Types = typename TypeListReverse<SrcTypes>::ValueT;

		private:
			TupleLikeObject_&&		_src;

		public:
			TupleReverser(TupleLikeObject_&& src) : _src(std::forward<TupleLikeObject_>(src)) { }

			template < size_t Index >
			const typename GetTypeListItem<Types, Index>::ValueT& Get() const &
			{ return _src.template Get<GetTypeListLength<SrcTypes>::Value - (Index + 1)>(); }

			template < size_t Index >
			typename EnableIf<IsNonConstRvalueReference<TupleLikeObject_&&>::Value, typename GetTypeListItem<Types, Index>::ValueT>::ValueT&& Get() &&
			{ return std::move(_src).template Get<GetTypeListLength<SrcTypes>::Value - (Index + 1)>(); }
		};

	}


	template < typename TupleLikeObject_ >
	Tuple<typename TypeListReverse<typename Decay<TupleLikeObject_>::ValueT::Types>::ValueT> ReverseTuple(TupleLikeObject_&& src)
	{ return Tuple<typename TypeListReverse<typename Decay<TupleLikeObject_>::ValueT::Types>::ValueT>::CreateFromTupleLikeObject(Detail::TupleReverser<TupleLikeObject_>(std::forward<TupleLikeObject_>(src))); }


	namespace Detail
	{

		template < typename TupleLikeObject_, template <typename> class Predicate_ >
		class TupleFilter
		{
			using SrcTypes = typename Decay<TupleLikeObject_>::ValueT::Types;

			template < typename Type, size_t InvertedIndex >
			using ToTypeWithIndex = TypeList<Type, IntToType<GetTypeListLength<SrcTypes>::Value - InvertedIndex - 1>>;

			template < typename Type, typename ResultTypes >
			using ToMyType = TypeListMerge<TypeList<TypeList<ToTypeWithIndex<Type, GetTypeListLength<ResultTypes>::Value>>, ResultTypes>>;

			template < typename TypeWithIndex >
			using ToType = GetTypeListItem<TypeWithIndex, 0>;

			template < typename TypeWithIndex >
			using ToIndex = GetTypeListItem<TypeWithIndex, 1>;

			template < typename TypeWithIndex >
			using MyPredicate = Predicate_<typename ToType<TypeWithIndex>::ValueT>;

			using SrcTypesWithIndexes = typename TypeListAccumulate<SrcTypes, ToMyType, TypeList<>>::ValueT;
			using MyTypesWithIndexes = typename TypeListCopyIf<SrcTypesWithIndexes, MyPredicate>::ValueT;

		public:
			using Types = typename TypeListTransform<MyTypesWithIndexes, ToType>::ValueT;

		private:
			TupleLikeObject_&&		_src;

		public:
			TupleFilter(TupleLikeObject_&& src) : _src(std::forward<TupleLikeObject_>(src)) { }

			template < size_t Index >
			const typename GetTypeListItem<Types, Index>::ValueT& Get() const &
			{ return _src.template Get<ToIndex<typename GetTypeListItem<MyTypesWithIndexes, Index>::ValueT>::ValueT::Value>(); }

			template < size_t Index >
			typename EnableIf<IsNonConstRvalueReference<TupleLikeObject_&&>::Value, typename GetTypeListItem<Types, Index>::ValueT>::ValueT&& Get() &&
			{ return std::move(_src).template Get<ToIndex<typename GetTypeListItem<MyTypesWithIndexes, Index>::ValueT>::ValueT::Value>(); }
		};

	}


	template < template <typename> class Predicate_, typename TupleLikeObject_ >
	Tuple<typename TypeListCopyIf<typename Decay<TupleLikeObject_>::ValueT::Types, Predicate_>::ValueT> FilterTuple(TupleLikeObject_&& src)
	{ return Tuple<typename TypeListCopyIf<typename Decay<TupleLikeObject_>::ValueT::Types, Predicate_>::ValueT>::CreateFromTupleLikeObject(Detail::TupleFilter<TupleLikeObject_, Predicate_>(std::forward<TupleLikeObject_>(src))); }

}

#endif
