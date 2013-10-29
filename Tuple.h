#ifndef STINGRAY_TOOLKIT_TUPLE_H
#define STINGRAY_TOOLKIT_TUPLE_H


#include <stingray/toolkit/Dummy.h>
#include <stingray/toolkit/toolkit.h>
#include <stingray/toolkit/TypeList.h>


namespace stingray
{


	template < typename T >
	struct GetParamPassingType
	{
		typedef typename Dereference<typename Deconst<T>::ValueT>::ValueT RawType;
		typedef typename If<IsBuiltinType<T>::Value || IsPointer<T>::Value, RawType, const RawType&>::ValueT ConstPassingType;
		typedef typename If<IsNonConstReference<T>::Value, RawType&, ConstPassingType>::ValueT ValueT;
	};

	namespace Detail
	{
		template < typename Tuple_, size_t Index >
		struct TupleItemGetter;
	}


	template < typename TupleLikeObject >
	class TupleShifter
	{
	public:
		typedef typename TupleLikeObject::TypeList::Next	TypeList;

	private:
		const TupleLikeObject&	_tlo;

	public:
		TupleShifter(const TupleLikeObject& tlo) : _tlo(tlo) { }

		template < size_t Index >
		inline typename GetParamPassingType<typename GetTypeListItem<typename TupleLikeObject::TypeList, Index + 1>::ValueT>::ValueT
		Get() const { return _tlo.template Get<Index + 1>(); }
	};


	struct TupleConstructorTag {};

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
		inline Tuple(const TupleConstructorTag& tag, const TupleLikeObject& tupleLikeObject)
			: _val(tupleLikeObject.template Get<0>()), _tail(tag, TupleShifter<TupleLikeObject>(tupleLikeObject))
		{ }

		template < typename TupleLikeObject >
		static Tuple CreateFromTupleLikeObject(const TupleLikeObject& tll)
		{ return Tuple(TupleConstructorTag(), tll); }

		inline Tuple(): _val(), _tail() {}

		inline Tuple(const Tuple& other) : _val(other._val), _tail(other._tail) { }

		inline Tuple(typename GetParamPassingType<ValueType>::ValueT p1) : _val(p1), _tail() { }

		inline Tuple(typename GetParamPassingType<ValueType>::ValueT head, const Tail& tail) : _val(head), _tail(tail) { }

#define TY typename
#define P_(N) typename GetParamPassingType<typename TryGetTypeListItem<TypeList, N - 1>::ValueT>::ValueT p##N

#define DETAIL_TOOLKIT_DECLARE_TUPLE_CTOR(Typenames_, ParamsDecl_, Params_) \
		inline Tuple(typename GetParamPassingType<ValueType>::ValueT p1, ParamsDecl_) : _val(p1), _tail(Params_) { }

		DETAIL_TOOLKIT_DECLARE_TUPLE_CTOR(MK_PARAM(TY T2), MK_PARAM(P_(2)), MK_PARAM(p2))
		DETAIL_TOOLKIT_DECLARE_TUPLE_CTOR(MK_PARAM(TY T2, TY T3), MK_PARAM(P_(2), P_(3)), MK_PARAM(p2, p3))
		DETAIL_TOOLKIT_DECLARE_TUPLE_CTOR(MK_PARAM(TY T2, TY T3, TY T4), MK_PARAM(P_(2), P_(3), P_(4)), MK_PARAM(p2, p3, p4))
		DETAIL_TOOLKIT_DECLARE_TUPLE_CTOR(MK_PARAM(TY T2, TY T3, TY T4, TY T5), MK_PARAM(P_(2), P_(3), P_(4), P_(5)), MK_PARAM(p2, p3, p4, p5))
		DETAIL_TOOLKIT_DECLARE_TUPLE_CTOR(MK_PARAM(TY T2, TY T3, TY T4, TY T5, TY T6), MK_PARAM(P_(2), P_(3), P_(4), P_(5), P_(6)), MK_PARAM(p2, p3, p4, p5, p6))
		DETAIL_TOOLKIT_DECLARE_TUPLE_CTOR(MK_PARAM(TY T2, TY T3, TY T4, TY T5, TY T6, TY T7), MK_PARAM(P_(2), P_(3), P_(4), P_(5), P_(6), P_(7)), MK_PARAM(p2, p3, p4, p5, p6, p7))
		DETAIL_TOOLKIT_DECLARE_TUPLE_CTOR(MK_PARAM(TY T2, TY T3, TY T4, TY T5, TY T6, TY T7, TY T8), MK_PARAM(P_(2), P_(3), P_(4), P_(5), P_(6), P_(7), P_(8)), MK_PARAM(p2, p3, p4, p5, p6, p7, p8))
		DETAIL_TOOLKIT_DECLARE_TUPLE_CTOR(MK_PARAM(TY T2, TY T3, TY T4, TY T5, TY T6, TY T7, TY T8, TY T9), MK_PARAM(P_(2), P_(3), P_(4), P_(5), P_(6), P_(7), P_(8), P_(9)), MK_PARAM(p2, p3, p4, p5, p6, p7, p8, p9))
		DETAIL_TOOLKIT_DECLARE_TUPLE_CTOR(MK_PARAM(TY T2, TY T3, TY T4, TY T5, TY T6, TY T7, TY T8, TY T9, TY T10), MK_PARAM(P_(2), P_(3), P_(4), P_(5), P_(6), P_(7), P_(8), P_(9), P_(10)), MK_PARAM(p2, p3, p4, p5, p6, p7, p8, p9, p10))
		DETAIL_TOOLKIT_DECLARE_TUPLE_CTOR(MK_PARAM(TY T2, TY T3, TY T4, TY T5, TY T6, TY T7, TY T8, TY T9, TY T10, TY T11), MK_PARAM(P_(2), P_(3), P_(4), P_(5), P_(6), P_(7), P_(8), P_(9), P_(10), P_(11)), MK_PARAM(p2, p3, p4, p5, p6, p7, p8, p9, p10, p11))
		DETAIL_TOOLKIT_DECLARE_TUPLE_CTOR(MK_PARAM(TY T2, TY T3, TY T4, TY T5, TY T6, TY T7, TY T8, TY T9, TY T10, TY T11, TY T12), MK_PARAM(P_(2), P_(3), P_(4), P_(5), P_(6), P_(7), P_(8), P_(9), P_(10), P_(11), P_(12)), MK_PARAM(p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12))
		DETAIL_TOOLKIT_DECLARE_TUPLE_CTOR(MK_PARAM(TY T2, TY T3, TY T4, TY T5, TY T6, TY T7, TY T8, TY T9, TY T10, TY T11, TY T12, TY 13), MK_PARAM(P_(2), P_(3), P_(4), P_(5), P_(6), P_(7), P_(8), P_(9), P_(10), P_(11), P_(12), P_(13)), MK_PARAM(p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13))
		DETAIL_TOOLKIT_DECLARE_TUPLE_CTOR(MK_PARAM(TY T2, TY T3, TY T4, TY T5, TY T6, TY T7, TY T8, TY T9, TY T10, TY T11, TY T12, TY 13, TY T14), MK_PARAM(P_(2), P_(3), P_(4), P_(5), P_(6), P_(7), P_(8), P_(9), P_(10), P_(11), P_(12), P_(13), P_(14)), MK_PARAM(p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14))
		DETAIL_TOOLKIT_DECLARE_TUPLE_CTOR(MK_PARAM(TY T2, TY T3, TY T4, TY T5, TY T6, TY T7, TY T8, TY T9, TY T10, TY T11, TY T12, TY 13, TY T14, TY T15), MK_PARAM(P_(2), P_(3), P_(4), P_(5), P_(6), P_(7), P_(8), P_(9), P_(10), P_(11), P_(12), P_(13), P_(14), P_(15)), MK_PARAM(p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15))


		inline typename GetParamPassingType<const ValueType>::ValueT GetHead() const { return _val; }
		inline typename Dereference<ValueType>::ValueT& GetHead() { return _val; }
		inline void SetHead(typename GetConstReferenceType<ValueType>::ValueT val) { _val = val; }
		inline const Tail& GetTail() const { return _tail; }
		inline Tail& GetTail() { return _tail; }

		template < size_t Index >
		inline typename GetParamPassingType<typename GetTypeListItem<TypeList, Index>::ValueT>::ValueT Get() const
		{ return Detail::TupleItemGetter<Tuple, Index>::Get(*this); }

		template < size_t Index >
		inline typename GetTypeListItem<TypeList, Index>::ValueT& Get()
		{ return Detail::TupleItemGetter<Tuple, Index>::Get(*this); }

		template < typename Type_ >
		inline typename GetParamPassingType<Type_>::ValueT Get(Dummy dummy = Dummy()) const
		{ return Detail::TupleItemGetter<Tuple, IndexOfTypeListItem<TypeList, Type_>::Value>::Get(*this); }

		template < typename Type_ >
		inline Type_& Get(Dummy dummy = Dummy())
		{ return Detail::TupleItemGetter<Tuple, IndexOfTypeListItem<TypeList, Type_>::Value>::Get(*this); }

		template < typename Type_, size_t Index >
		inline typename GetParamPassingType<Type_>::ValueT Get(Dummy dummy = Dummy(), Dummy dummy2 = Dummy()) const
		{ return Detail::TupleItemGetter<Tuple, IndexOfTypeListItem<TypeList, Type_, Index>::Value>::Get(*this); }

		template < typename Type_, size_t Index >
		inline Type_& Get(Dummy dummy = Dummy(), Dummy dummy2 = Dummy())
		{ return Detail::TupleItemGetter<Tuple, IndexOfTypeListItem<TypeList, Type_, Index>::Value>::Get(*this); }

		template < size_t Index >
		inline void Set(typename GetConstReferenceType<typename GetTypeListItem<TypeList, Index>::ValueT>::ValueT val)
		{ Detail::TupleItemGetter<Tuple, Index>::Set(*this, val); }
	};


	template < >
	class Tuple<TypeListEndNode>
	{
	public:
		static const size_t Size = 0;

		inline Tuple() { }

		template < typename TupleLikeObject >
		inline Tuple(const TupleConstructorTag& tag, const TupleLikeObject& tupleLikeObject)
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
			static inline typename GetParamPassingType<typename GetTypeListItem<typename Tuple_::TypeList, Index>::ValueT>::ValueT
			Get(const Tuple_& tuple)
			{ return TupleItemGetter<typename Tuple_::Tail, Index - 1>::Get(tuple.GetTail()); }

			static inline typename Dereference<typename GetTypeListItem<typename Tuple_::TypeList, Index>::ValueT>::ValueT&
			Get(Tuple_& tuple)
			{ return TupleItemGetter<typename Tuple_::Tail, Index - 1>::Get(tuple.GetTail()); }

			static inline void Set(Tuple_& tuple, typename GetConstReferenceType<typename GetTypeListItem<typename Tuple_::TypeList, Index>::ValueT>::ValueT val)
			{ TupleItemGetter<typename Tuple_::Tail, Index - 1>::Set(tuple.GetTail(), val); }
		};

		template < typename Tuple_ >
		struct TupleItemGetter<Tuple_, 0>
		{
			static inline typename GetParamPassingType<typename Tuple_::ValueType>::ValueT
			Get(const Tuple_& tuple)
			{ return tuple.GetHead(); }

			static inline typename Dereference<typename Tuple_::ValueType>::ValueT&
			Get(Tuple_& tuple)
			{ return tuple.GetHead(); }

			static inline void Set(Tuple_& tuple, typename GetConstReferenceType<typename Tuple_::ValueType>::ValueT val)
			{ tuple.SetHead(val); }
		};
	}


	template < size_t Index, typename Tuple_ >
	inline typename GetParamPassingType<typename GetTypeListItem<typename Tuple_::TypeList, Index>::ValueT>::ValueT GetTupleItem(const Tuple_& tuple)
	{ return Detail::TupleItemGetter<Tuple_, Index>::Get(tuple); }

#undef P_
#define P_(N) const T##N & p##N

#define DETAIL_TOOLKIT_DECLARE_MAKETUPLE(N_, TypesDecl_, TypesUsage_, ParamsDecl_, ParamsUsage_) \
	template < TypesDecl_ > \
	Tuple<TypeList_##N_<TypesUsage_> > MakeTuple(ParamsDecl_) \
	{ return Tuple<TypeList_##N_<TypesUsage_> >(ParamsUsage_); }

	DETAIL_TOOLKIT_DECLARE_MAKETUPLE(1, MK_PARAM(TY T1), MK_PARAM(T1), MK_PARAM(P_(1)), MK_PARAM(p1));
	DETAIL_TOOLKIT_DECLARE_MAKETUPLE(2, MK_PARAM(TY T1, TY T2), MK_PARAM(T1, T2), MK_PARAM(P_(1), P_(2)), MK_PARAM(p1, p2));
	DETAIL_TOOLKIT_DECLARE_MAKETUPLE(3, MK_PARAM(TY T1, TY T2, TY T3), MK_PARAM(T1, T2, T3), MK_PARAM(P_(1), P_(2), P_(3)), MK_PARAM(p1, p2, p3));
	DETAIL_TOOLKIT_DECLARE_MAKETUPLE(4, MK_PARAM(TY T1, TY T2, TY T3, TY T4), MK_PARAM(T1, T2, T3, T4), MK_PARAM(P_(1), P_(2), P_(3), P_(4)), MK_PARAM(p1, p2, p3, p4));
	DETAIL_TOOLKIT_DECLARE_MAKETUPLE(5, MK_PARAM(TY T1, TY T2, TY T3, TY T4, TY T5), MK_PARAM(T1, T2, T3, T4, T5), MK_PARAM(P_(1), P_(2), P_(3), P_(4), P_(5)), MK_PARAM(p1, p2, p3, p4, p5));

#undef DETAIL_TOOLKIT_DECLARE_MAKETUPLE
#undef P_
#undef TY


}


#endif
