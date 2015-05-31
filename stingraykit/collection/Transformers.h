#ifndef STINGRAYKIT_COLLECTION_TRANSFORMERS_H
#define STINGRAYKIT_COLLECTION_TRANSFORMERS_H

namespace stingray
{

	struct TransformerMarker
	{
	protected:
		TransformerMarker()
		{ }
	};


	template<typename T>
	struct IsTransformer
	{ static const bool Value = Inherits<T, TransformerMarker>::Value; };


	template <typename Arg_, typename Transformer_>
	typename EnableIf<IsTransformer<Transformer_>::Value, typename Transformer_::template Dispatcher<Arg_>::Impl::ValueT>::ValueT operator | (const Arg_& arg, const Transformer_& action)
	{ return Transformer_::template Dispatcher<Arg_>::Impl::Do(arg, action); }


	template<typename Arg_, typename Predicate_, typename Enabler = void>
	struct FilterTransformerImpl;


	template<typename Predicate_>
	struct FilterTransformer : public TransformerMarker
	{
		template<typename Arg_>
		struct Dispatcher
		{
			typedef FilterTransformerImpl<Arg_, Predicate_> Impl;
		};

	private:
		Predicate_ _predicate;

	public:
		FilterTransformer(const Predicate_& predicate) : _predicate(predicate)
		{ }

		Predicate_ GetPredicate() const
		{ return _predicate; }
	};


	template<typename Predicate_>
	FilterTransformer<Predicate_> Filter(const Predicate_& predicate)
	{ return FilterTransformer<Predicate_>(predicate); }


	template<typename Arg_, typename Enabler = void>
	struct ReverseTransformerImpl;


	struct ReverseTransformer : public TransformerMarker
	{
		template<typename Arg_>
		struct Dispatcher
		{
			typedef ReverseTransformerImpl<Arg_> Impl;
		};
	};


	inline ReverseTransformer Reverse()
	{ return ReverseTransformer(); }


	template<typename Arg_, typename Functor_, typename Enabler = void>
	struct TransformTransformerImpl;


	template<typename Functor_>
	struct TransformTransformer : public TransformerMarker
	{
		template<typename Arg_>
		struct Dispatcher
		{
			typedef TransformTransformerImpl<Arg_, Functor_> Impl;
		};

	private:
		Functor_ _functor;

	public:
		TransformTransformer(const Functor_& functor) : _functor(functor)
		{ }

		Functor_ GetFunctor() const
		{ return _functor; }
	};


	template<typename Functor_>
	TransformTransformer<Functor_> Transform(const Functor_& functor)
	{ return TransformTransformer<Functor_>(functor); }


	template<typename Arg_, typename Dst_, typename Enabler = void>
	struct CastTransformerImpl;


	template<typename Dst_>
	struct CastTransformer : public TransformerMarker
	{
		template<typename Arg_>
		struct Dispatcher
		{
			typedef CastTransformerImpl<Arg_, Dst_> Impl;
		};
	};


	template<typename Dst_>
	CastTransformer<Dst_> Cast()
	{ return CastTransformer<Dst_>(); }


	template<typename Arg_, typename Dst_, typename Enabler = void>
	struct OfTypeTransformerImpl;


	template<typename Dst_>
	struct OfTypeTransformer : public TransformerMarker
	{
		template<typename Arg_>
		struct Dispatcher
		{
			typedef OfTypeTransformerImpl<Arg_, Dst_> Impl;
		};
	};


	template<typename Dst_>
	OfTypeTransformer<Dst_> OfType()
	{ return OfTypeTransformer<Dst_>(); }

}

#endif
