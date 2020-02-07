#ifndef STINGRAYKIT_TUPLE_H
#define STINGRAYKIT_TUPLE_H

// Copyright (c) 2011 - 2019, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/core/Dummy.h>
#include <stingraykit/metaprogramming/ParamPassingType.h>
#include <stingraykit/Types.h>

namespace stingray
{

	namespace Detail
	{
		template < typename Tuple_, size_t Index >
		struct TupleItemGetter;
	}


	struct TupleConstructorTag { };

	template < typename TypeList_ >
	class Tuple
	{
	public:
		typedef TypeList_						TypeList;
		typedef Tuple<typename TypeList::Next>	Tail;
		typedef typename TypeList::ValueT		ValueType;

		static const size_t Size = Tail::Size + 1;

	private:
		ValueType	_val;
		Tail		_tail;

	public:
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

		Tuple() : _val(), _tail() { }

		Tuple(typename GetParamPassingType<ValueType>::ValueT p1) : _val(p1), _tail() { }

		Tuple(typename GetParamPassingType<ValueType>::ValueT head, const Tail& tail) : _val(head), _tail(tail) { }

#define TY typename
#define P_(N) typename GetParamPassingType<typename TryGetTypeListItem<TypeList, N - 1>::ValueT>::ValueT p##N

#define DETAIL_STINGRAYKIT_DECLARE_TUPLE_CTOR(Typenames_, ParamsDecl_, Params_) \
		Tuple(typename GetParamPassingType<ValueType>::ValueT p1, ParamsDecl_) : _val(p1), _tail(Params_) { }

		DETAIL_STINGRAYKIT_DECLARE_TUPLE_CTOR(MK_PARAM(TY T2), MK_PARAM(P_(2)), MK_PARAM(p2))
		DETAIL_STINGRAYKIT_DECLARE_TUPLE_CTOR(MK_PARAM(TY T2, TY T3), MK_PARAM(P_(2), P_(3)), MK_PARAM(p2, p3))
		DETAIL_STINGRAYKIT_DECLARE_TUPLE_CTOR(MK_PARAM(TY T2, TY T3, TY T4), MK_PARAM(P_(2), P_(3), P_(4)), MK_PARAM(p2, p3, p4))
		DETAIL_STINGRAYKIT_DECLARE_TUPLE_CTOR(MK_PARAM(TY T2, TY T3, TY T4, TY T5), MK_PARAM(P_(2), P_(3), P_(4), P_(5)), MK_PARAM(p2, p3, p4, p5))
		DETAIL_STINGRAYKIT_DECLARE_TUPLE_CTOR(MK_PARAM(TY T2, TY T3, TY T4, TY T5, TY T6), MK_PARAM(P_(2), P_(3), P_(4), P_(5), P_(6)), MK_PARAM(p2, p3, p4, p5, p6))
		DETAIL_STINGRAYKIT_DECLARE_TUPLE_CTOR(MK_PARAM(TY T2, TY T3, TY T4, TY T5, TY T6, TY T7), MK_PARAM(P_(2), P_(3), P_(4), P_(5), P_(6), P_(7)), MK_PARAM(p2, p3, p4, p5, p6, p7))
		DETAIL_STINGRAYKIT_DECLARE_TUPLE_CTOR(MK_PARAM(TY T2, TY T3, TY T4, TY T5, TY T6, TY T7, TY T8), MK_PARAM(P_(2), P_(3), P_(4), P_(5), P_(6), P_(7), P_(8)), MK_PARAM(p2, p3, p4, p5, p6, p7, p8))
		DETAIL_STINGRAYKIT_DECLARE_TUPLE_CTOR(MK_PARAM(TY T2, TY T3, TY T4, TY T5, TY T6, TY T7, TY T8, TY T9), MK_PARAM(P_(2), P_(3), P_(4), P_(5), P_(6), P_(7), P_(8), P_(9)), MK_PARAM(p2, p3, p4, p5, p6, p7, p8, p9))
		DETAIL_STINGRAYKIT_DECLARE_TUPLE_CTOR(MK_PARAM(TY T2, TY T3, TY T4, TY T5, TY T6, TY T7, TY T8, TY T9, TY T10), MK_PARAM(P_(2), P_(3), P_(4), P_(5), P_(6), P_(7), P_(8), P_(9), P_(10)), MK_PARAM(p2, p3, p4, p5, p6, p7, p8, p9, p10))
		DETAIL_STINGRAYKIT_DECLARE_TUPLE_CTOR(MK_PARAM(TY T2, TY T3, TY T4, TY T5, TY T6, TY T7, TY T8, TY T9, TY T10, TY T11), MK_PARAM(P_(2), P_(3), P_(4), P_(5), P_(6), P_(7), P_(8), P_(9), P_(10), P_(11)), MK_PARAM(p2, p3, p4, p5, p6, p7, p8, p9, p10, p11))
		DETAIL_STINGRAYKIT_DECLARE_TUPLE_CTOR(MK_PARAM(TY T2, TY T3, TY T4, TY T5, TY T6, TY T7, TY T8, TY T9, TY T10, TY T11, TY T12), MK_PARAM(P_(2), P_(3), P_(4), P_(5), P_(6), P_(7), P_(8), P_(9), P_(10), P_(11), P_(12)), MK_PARAM(p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12))
		DETAIL_STINGRAYKIT_DECLARE_TUPLE_CTOR(MK_PARAM(TY T2, TY T3, TY T4, TY T5, TY T6, TY T7, TY T8, TY T9, TY T10, TY T11, TY T12, TY 13), MK_PARAM(P_(2), P_(3), P_(4), P_(5), P_(6), P_(7), P_(8), P_(9), P_(10), P_(11), P_(12), P_(13)), MK_PARAM(p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13))
		DETAIL_STINGRAYKIT_DECLARE_TUPLE_CTOR(MK_PARAM(TY T2, TY T3, TY T4, TY T5, TY T6, TY T7, TY T8, TY T9, TY T10, TY T11, TY T12, TY 13, TY T14), MK_PARAM(P_(2), P_(3), P_(4), P_(5), P_(6), P_(7), P_(8), P_(9), P_(10), P_(11), P_(12), P_(13), P_(14)), MK_PARAM(p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14))
		DETAIL_STINGRAYKIT_DECLARE_TUPLE_CTOR(MK_PARAM(TY T2, TY T3, TY T4, TY T5, TY T6, TY T7, TY T8, TY T9, TY T10, TY T11, TY T12, TY 13, TY T14, TY T15), MK_PARAM(P_(2), P_(3), P_(4), P_(5), P_(6), P_(7), P_(8), P_(9), P_(10), P_(11), P_(12), P_(13), P_(14), P_(15)), MK_PARAM(p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15))
		DETAIL_STINGRAYKIT_DECLARE_TUPLE_CTOR(MK_PARAM(TY T2, TY T3, TY T4, TY T5, TY T6, TY T7, TY T8, TY T9, TY T10, TY T11, TY T12, TY 13, TY T14, TY T15, TY T16), MK_PARAM(P_(2), P_(3), P_(4), P_(5), P_(6), P_(7), P_(8), P_(9), P_(10), P_(11), P_(12), P_(13), P_(14), P_(15), P_(16)), MK_PARAM(p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16))
		DETAIL_STINGRAYKIT_DECLARE_TUPLE_CTOR(MK_PARAM(TY T2, TY T3, TY T4, TY T5, TY T6, TY T7, TY T8, TY T9, TY T10, TY T11, TY T12, TY 13, TY T14, TY T15, TY T16, TY T17), MK_PARAM(P_(2), P_(3), P_(4), P_(5), P_(6), P_(7), P_(8), P_(9), P_(10), P_(11), P_(12), P_(13), P_(14), P_(15), P_(16), P_(17)), MK_PARAM(p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17))
		DETAIL_STINGRAYKIT_DECLARE_TUPLE_CTOR(MK_PARAM(TY T2, TY T3, TY T4, TY T5, TY T6, TY T7, TY T8, TY T9, TY T10, TY T11, TY T12, TY 13, TY T14, TY T15, TY T16, TY T17, TY T18), MK_PARAM(P_(2), P_(3), P_(4), P_(5), P_(6), P_(7), P_(8), P_(9), P_(10), P_(11), P_(12), P_(13), P_(14), P_(15), P_(16), P_(17), P_(18)), MK_PARAM(p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18))
		DETAIL_STINGRAYKIT_DECLARE_TUPLE_CTOR(MK_PARAM(TY T2, TY T3, TY T4, TY T5, TY T6, TY T7, TY T8, TY T9, TY T10, TY T11, TY T12, TY 13, TY T14, TY T15, TY T16, TY T17, TY T18, TY T19), MK_PARAM(P_(2), P_(3), P_(4), P_(5), P_(6), P_(7), P_(8), P_(9), P_(10), P_(11), P_(12), P_(13), P_(14), P_(15), P_(16), P_(17), P_(18), P_(19)), MK_PARAM(p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19))
		DETAIL_STINGRAYKIT_DECLARE_TUPLE_CTOR(MK_PARAM(TY T2, TY T3, TY T4, TY T5, TY T6, TY T7, TY T8, TY T9, TY T10, TY T11, TY T12, TY 13, TY T14, TY T15, TY T16, TY T17, TY T18, TY T19, TY T20), MK_PARAM(P_(2), P_(3), P_(4), P_(5), P_(6), P_(7), P_(8), P_(9), P_(10), P_(11), P_(12), P_(13), P_(14), P_(15), P_(16), P_(17), P_(18), P_(19), P_(20)), MK_PARAM(p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20))


		typename GetParamPassingType<ValueType>::ValueT GetHead() const { return _val; }
		typename RemoveReference<ValueType>::ValueT& GetHead() { return _val; }
		void SetHead(typename AddConstReference<ValueType>::ValueT val) { _val = val; }
		const Tail& GetTail() const { return _tail; }
		Tail& GetTail() { return _tail; }

		template < size_t Index >
		typename GetParamPassingType<typename GetTypeListItem<TypeList, Index>::ValueT>::ValueT Get() const
		{ return Detail::TupleItemGetter<Tuple, Index>::Get(*this); }

		template < size_t Index >
		typename GetTypeListItem<TypeList, Index>::ValueT& Get()
		{ return Detail::TupleItemGetter<Tuple, Index>::Get(*this); }

		template < typename Type_ >
		typename GetParamPassingType<Type_>::ValueT Get(Dummy dummy = Dummy()) const
		{ return Detail::TupleItemGetter<Tuple, IndexOfTypeListItem<TypeList, Type_>::Value>::Get(*this); }

		template < typename Type_ >
		Type_& Get(Dummy dummy = Dummy())
		{ return Detail::TupleItemGetter<Tuple, IndexOfTypeListItem<TypeList, Type_>::Value>::Get(*this); }

		template < typename Type_, size_t Index >
		typename GetParamPassingType<Type_>::ValueT Get(Dummy dummy = Dummy(), Dummy dummy2 = Dummy()) const
		{ return Detail::TupleItemGetter<Tuple, IndexOfTypeListItem<TypeList, Type_, Index>::Value>::Get(*this); }

		template < typename Type_, size_t Index >
		Type_& Get(Dummy dummy = Dummy(), Dummy dummy2 = Dummy())
		{ return Detail::TupleItemGetter<Tuple, IndexOfTypeListItem<TypeList, Type_, Index>::Value>::Get(*this); }

		template < size_t Index >
		void Set(typename AddConstReference<typename GetTypeListItem<TypeList, Index>::ValueT>::ValueT val)
		{ Detail::TupleItemGetter<Tuple, Index>::Set(*this, val); }
	};


	template < >
	class Tuple<TypeListEndNode>
	{
	public:
		typedef TypeList<>::type				TypeList;

		static const size_t Size = 0;

		Tuple() { }

		template < typename TupleLikeObject, typename IndexOffset >
		Tuple(const TupleConstructorTag& tag, const TupleLikeObject& tupleLikeObject, IndexOffset Dummy)
		{ CompileTimeAssert<GetTypeListLength<typename TupleLikeObject::TypeList>::Value == IndexOffset::Value> ERROR__tuple_like_object_is_too_big; }

		template < typename TupleLikeObject >
		Tuple(const TupleConstructorTag& tag, const TupleLikeObject& tupleLikeObject)
		{ CompileTimeAssert<GetTypeListLength<typename TupleLikeObject::TypeList>::Value == 0> ERROR__tuple_like_object_is_too_big; }

		template < typename TupleLikeObject >
		static Tuple CreateFromTupleLikeObject(const TupleLikeObject& tll)
		{ return Tuple(TupleConstructorTag(), tll); }
	};


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
			typename GetParamPassingType<typename GetTypeListItem<TypeList, Index>::ValueT>::ValueT Get() const
			{ return _src.template Get<GetTypeListLength<SrcTypeList>::Value - (Index + 1)>(); }
		};
	}


	template < typename TupleLikeObject_ >
	Tuple<typename TypeListReverse<typename TupleLikeObject_::TypeList>::ValueT> ReverseTuple(const TupleLikeObject_& src)
	{ return Tuple<typename TypeListReverse<typename TupleLikeObject_::TypeList>::ValueT>::CreateFromTupleLikeObject(Detail::TupleReverser<TupleLikeObject_>(src)); }


	namespace Detail
	{
		template < typename Tuple_, size_t Index >
		struct TupleItemGetter
		{
			static typename GetParamPassingType<typename GetTypeListItem<typename Tuple_::TypeList, Index>::ValueT>::ValueT
			Get(const Tuple_& tuple)
			{ return TupleItemGetter<typename Tuple_::Tail, Index - 1>::Get(tuple.GetTail()); }

			static typename RemoveReference<typename GetTypeListItem<typename Tuple_::TypeList, Index>::ValueT>::ValueT&
			Get(Tuple_& tuple)
			{ return TupleItemGetter<typename Tuple_::Tail, Index - 1>::Get(tuple.GetTail()); }

			static void Set(Tuple_& tuple, typename AddConstReference<typename GetTypeListItem<typename Tuple_::TypeList, Index>::ValueT>::ValueT val)
			{ TupleItemGetter<typename Tuple_::Tail, Index - 1>::Set(tuple.GetTail(), val); }
		};

		template < typename Tuple_ >
		struct TupleItemGetter<Tuple_, 0>
		{
			static typename GetParamPassingType<typename Tuple_::ValueType>::ValueT
			Get(const Tuple_& tuple)
			{ return tuple.GetHead(); }

			static typename RemoveReference<typename Tuple_::ValueType>::ValueT&
			Get(Tuple_& tuple)
			{ return tuple.GetHead(); }

			static void Set(Tuple_& tuple, typename AddConstReference<typename Tuple_::ValueType>::ValueT val)
			{ tuple.SetHead(val); }
		};
	}


	template < size_t Index, typename Tuple_ >
	typename GetParamPassingType<typename GetTypeListItem<typename Tuple_::TypeList, Index>::ValueT>::ValueT GetTupleItem(const Tuple_& tuple)
	{ return Detail::TupleItemGetter<Tuple_, Index>::Get(tuple); }


	inline Tuple<TypeList<>::type> MakeTuple()
	{ return Tuple<TypeList<>::type>(); }


#undef P_
#define P_(N) const T##N & p##N

#define DETAIL_STINGRAYKIT_DECLARE_MAKETUPLE(N_, TypesDecl_, TypesUsage_, ParamsDecl_, ParamsUsage_) \
	template < TypesDecl_ > \
	Tuple<typename TypeList<TypesUsage_>::type> MakeTuple(ParamsDecl_) \
	{ return Tuple<typename TypeList<TypesUsage_>::type>(ParamsUsage_); }

	DETAIL_STINGRAYKIT_DECLARE_MAKETUPLE(1, MK_PARAM(TY T1), MK_PARAM(T1), MK_PARAM(P_(1)), MK_PARAM(p1));
	DETAIL_STINGRAYKIT_DECLARE_MAKETUPLE(2, MK_PARAM(TY T1, TY T2), MK_PARAM(T1, T2), MK_PARAM(P_(1), P_(2)), MK_PARAM(p1, p2));
	DETAIL_STINGRAYKIT_DECLARE_MAKETUPLE(3, MK_PARAM(TY T1, TY T2, TY T3), MK_PARAM(T1, T2, T3), MK_PARAM(P_(1), P_(2), P_(3)), MK_PARAM(p1, p2, p3));
	DETAIL_STINGRAYKIT_DECLARE_MAKETUPLE(4, MK_PARAM(TY T1, TY T2, TY T3, TY T4), MK_PARAM(T1, T2, T3, T4), MK_PARAM(P_(1), P_(2), P_(3), P_(4)), MK_PARAM(p1, p2, p3, p4));
	DETAIL_STINGRAYKIT_DECLARE_MAKETUPLE(5, MK_PARAM(TY T1, TY T2, TY T3, TY T4, TY T5), MK_PARAM(T1, T2, T3, T4, T5), MK_PARAM(P_(1), P_(2), P_(3), P_(4), P_(5)), MK_PARAM(p1, p2, p3, p4, p5));

#undef DETAIL_STINGRAYKIT_DECLARE_MAKETUPLE
#undef P_
#undef TY


}


#endif
