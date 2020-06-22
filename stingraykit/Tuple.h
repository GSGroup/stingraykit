#ifndef STINGRAYKIT_TUPLE_H
#define STINGRAYKIT_TUPLE_H

// Copyright (c) 2011 - 2019, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/core/Dummy.h>
#include <stingraykit/core/NonCopyable.h>
#include <stingraykit/metaprogramming/EnableIf.h>
#include <stingraykit/metaprogramming/TypeTraits.h>
#include <stingraykit/Types.h>

#include <utility>

namespace stingray
{

	namespace Detail
	{
		template < typename Tuple_, size_t Index >
		struct TupleItemGetter
		{
			static const typename GetTypeListItem<typename Tuple_::TypeList, Index>::ValueT& Get(const Tuple_& tuple)
			{ return TupleItemGetter<typename Tuple_::Tail, Index - 1>::Get(tuple.GetTail()); }

			static typename GetTypeListItem<typename Tuple_::TypeList, Index>::ValueT& Get(Tuple_& tuple)
			{ return TupleItemGetter<typename Tuple_::Tail, Index - 1>::Get(tuple.GetTail()); }
		};

		template < typename Tuple_ >
		struct TupleItemGetter<Tuple_, 0>
		{
			static const typename Tuple_::ValueType& Get(const Tuple_& tuple)
			{ return tuple.GetHead(); }

			static typename Tuple_::ValueType& Get(Tuple_& tuple)
			{ return tuple.GetHead(); }
		};
	}


	struct TupleConstructorTag { };

	template < typename TypeList_ >
	class Tuple
	{
		STINGRAYKIT_DEFAULTCOPYABLE(Tuple);
		STINGRAYKIT_DEFAULTMOVABLE(Tuple);

	public:
		typedef TypeList_						TypeList;
		typedef Tuple<typename TypeList::Next>	Tail;
		typedef typename TypeList::ValueT		ValueType;

		static const size_t Size = Tail::Size + 1;

	private:
		ValueType	_val;
		Tail		_tail;

	public:
		Tuple() : _val(), _tail() { }

		template < typename T0, typename... Ts, typename EnableIf<!IsSame<typename Decay<T0>::ValueT, TupleConstructorTag>::Value && !IsSame<typename Decay<T0>::ValueT, Tuple<TypeList>>::Value, bool>::ValueT = false >
		Tuple(T0&& p0, Ts&&... args)
			: _val(std::forward<T0>(p0)), _tail(std::forward<Ts>(args)...)
		{ }

		template < typename OtherTypeList_, typename EnableIf<!IsSame<Tuple<OtherTypeList_>, typename Decay<ValueType>::ValueT>::Value && !IsSame<OtherTypeList_, TypeListEndNode>::Value, bool>::ValueT = false >
		Tuple(const Tuple<OtherTypeList_>& other)
			: _val(other.GetHead()), _tail(other.GetTail())
		{ }

		template < typename OtherTypeList_, typename EnableIf<!IsSame<Tuple<OtherTypeList_>, typename Decay<ValueType>::ValueT>::Value && !IsSame<OtherTypeList_, TypeListEndNode>::Value, bool>::ValueT = false >
		Tuple(Tuple<OtherTypeList_>&& other)
			: _val(std::move(other.GetHead())), _tail(std::move(other.GetTail()))
		{ }

		template < typename TupleLikeObject >
		Tuple(const TupleConstructorTag& tag, const TupleLikeObject& tupleLikeObject)
			: _val(tupleLikeObject.template Get<0>()), _tail(tag, tupleLikeObject, integral_constant<size_t, 1>())
		{ }

		template < typename TupleLikeObject, typename IndexOffset >
		Tuple(const TupleConstructorTag& tag, const TupleLikeObject& tupleLikeObject, IndexOffset Dummy)
			: _val(tupleLikeObject.template Get<IndexOffset::Value>()), _tail(tag, tupleLikeObject, integral_constant<size_t, IndexOffset::Value + 1>())
		{ }

		template < typename TupleLikeObject >
		static Tuple CreateFromTupleLikeObject(const TupleLikeObject& tll)
		{ return Tuple(TupleConstructorTag(), tll); }

		const ValueType& GetHead() const { return _val; }
		ValueType& GetHead() { return _val; }

		const Tail& GetTail() const { return _tail; }
		Tail& GetTail() { return _tail; }

		template < size_t Index >
		const typename GetTypeListItem<TypeList, Index>::ValueT& Get() const
		{ return Detail::TupleItemGetter<Tuple, Index>::Get(*this); }

		template < size_t Index >
		typename GetTypeListItem<TypeList, Index>::ValueT& Get()
		{ return Detail::TupleItemGetter<Tuple, Index>::Get(*this); }

		template < typename Type_ >
		const Type_& Get(Dummy dummy = Dummy()) const
		{ return Detail::TupleItemGetter<Tuple, IndexOfTypeListItem<TypeList, Type_>::Value>::Get(*this); }

		template < typename Type_ >
		Type_& Get(Dummy dummy = Dummy())
		{ return Detail::TupleItemGetter<Tuple, IndexOfTypeListItem<TypeList, Type_>::Value>::Get(*this); }

		template < typename Type_, size_t Index >
		const Type_& Get(Dummy dummy = Dummy(), Dummy dummy2 = Dummy()) const
		{ return Detail::TupleItemGetter<Tuple, IndexOfTypeListItem<TypeList, Type_, Index>::Value>::Get(*this); }

		template < typename Type_, size_t Index >
		Type_& Get(Dummy dummy = Dummy(), Dummy dummy2 = Dummy())
		{ return Detail::TupleItemGetter<Tuple, IndexOfTypeListItem<TypeList, Type_, Index>::Value>::Get(*this); }
	};


	template < >
	class Tuple<TypeListEndNode>
	{
		STINGRAYKIT_DEFAULTCOPYABLE(Tuple<TypeListEndNode>);
		STINGRAYKIT_DEFAULTMOVABLE(Tuple<TypeListEndNode>);

	public:
		typedef TypeList<>::type				TypeList;

		static const size_t Size = 0;

		Tuple() { }

		template < typename TupleLikeObject >
		Tuple(const TupleConstructorTag& tag, const TupleLikeObject& tupleLikeObject)
		{ CompileTimeAssert<GetTypeListLength<typename TupleLikeObject::TypeList>::Value == 0> ERROR__tuple_like_object_is_too_big; }

		template < typename TupleLikeObject, typename IndexOffset >
		Tuple(const TupleConstructorTag& tag, const TupleLikeObject& tupleLikeObject, IndexOffset Dummy)
		{ CompileTimeAssert<GetTypeListLength<typename TupleLikeObject::TypeList>::Value == IndexOffset::Value> ERROR__tuple_like_object_is_too_big; }

		template < typename TupleLikeObject >
		static Tuple CreateFromTupleLikeObject(const TupleLikeObject& tll)
		{ return Tuple(TupleConstructorTag(), tll); }
	};


	template < typename... Ts >
	Tuple<typename TypeList<Ts...>::type> MakeTuple(Ts&&... args)
	{ return Tuple<typename TypeList<Ts...>::type>(std::forward<Ts>(args)...); }


	namespace Detail
	{
		template < typename TupleLikeObject_ >
		class TupleReverser
		{
		public:
			typedef typename TupleLikeObject_::TypeList				SrcTypeList;
			typedef typename TypeListReverse<SrcTypeList>::ValueT	TypeList;

		private:
			const TupleLikeObject_&		_src;

		public:
			TupleReverser(const TupleLikeObject_& src) : _src(src) { }

			template < size_t Index >
			const typename GetTypeListItem<TypeList, Index>::ValueT& Get() const
			{ return _src.template Get<GetTypeListLength<SrcTypeList>::Value - (Index + 1)>(); }
		};
	}


	template < typename TupleLikeObject_ >
	Tuple<typename TypeListReverse<typename TupleLikeObject_::TypeList>::ValueT> ReverseTuple(const TupleLikeObject_& src)
	{ return Tuple<typename TypeListReverse<typename TupleLikeObject_::TypeList>::ValueT>::CreateFromTupleLikeObject(Detail::TupleReverser<TupleLikeObject_>(src)); }

}

#endif
