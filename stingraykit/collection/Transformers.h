#ifndef STINGRAYKIT_COLLECTION_TRANSFORMERS_H
#define STINGRAYKIT_COLLECTION_TRANSFORMERS_H

// Copyright (c) 2011 - 2017, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/metaprogramming/EnableIf.h>
#include <stingraykit/metaprogramming/TypeRelationships.h>

namespace stingray
{

	struct TransformerMarker
	{
	protected:
		TransformerMarker()
		{ }
	};


	template<typename T>
	struct IsTransformer : IsInherited<T, TransformerMarker> { };


	template <typename Arg_, typename Enabler = void>
	struct FirstTransformerImpl;


	struct FirstTransformer : public TransformerMarker
	{
		template<typename Arg_>
		struct Dispatcher
		{
			typedef FirstTransformerImpl<Arg_> Impl;
		};
	};


	inline FirstTransformer First()
	{ return FirstTransformer(); }


	template<typename Arg_, typename Enabler = void>
	struct FirstOrDefaultTransformerImpl;


	struct FirstOrDefaultTransformer : public TransformerMarker
	{
		template<typename Arg_>
		struct Dispatcher
		{
			typedef FirstOrDefaultTransformerImpl<Arg_> Impl;
		};
	};


	inline FirstOrDefaultTransformer FirstOrDefault()
	{ return FirstOrDefaultTransformer(); }


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


	template<typename Arg_, typename Enabler = void>
	struct AnyTransformerImpl;


	struct AnyTransformer : public TransformerMarker
	{
		template<typename Arg_>
		struct Dispatcher
		{
			typedef AnyTransformerImpl<Arg_> Impl;
		};
	};


	inline AnyTransformer Any()
	{ return AnyTransformer(); }


	template <typename Arg_, typename Enabler = void>
	struct CountTransformerImpl;


	struct CountTransformer : public TransformerMarker
	{
		template <typename Arg_>
		struct Dispatcher
		{
			typedef CountTransformerImpl<Arg_> Impl;
		};
	};


	inline CountTransformer Count()
	{ return CountTransformer(); }

}

#endif
