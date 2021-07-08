#ifndef STINGRAYKIT_COLLECTION_ENUMERABLEHELPERS_H
#define STINGRAYKIT_COLLECTION_ENUMERABLEHELPERS_H

// Copyright (c) 2011 - 2019, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/collection/EnumerableWrapper.h>
#include <stingraykit/collection/EnumeratorFromStlContainer.h>
#include <stingraykit/collection/Transformers.h>
#include <stingraykit/compare/Comparable.h>
#include <stingraykit/function/bind.h>
#include <stingraykit/RefStorage.h>

namespace stingray
{

	/**
	 * @addtogroup toolkit_collections
	 * @{
	 */

	namespace Detail
	{
		template < typename T >
		struct EmptyEnumerator : public virtual IEnumerator<T>
		{
			virtual bool Valid() const	{ return false; }
			virtual T Get() const		{ STINGRAYKIT_THROW("Enumerator is not valid!"); }
			virtual void Next()			{ STINGRAYKIT_THROW("Enumerator is not valid!"); }
		};

		class EmptyEnumeratorProxy
		{
		public:
			template < typename U >
			operator shared_ptr<IEnumerator<U> > () const
			{ return make_shared_ptr<EmptyEnumerator<U> >(); }
		};
	}


	inline Detail::EmptyEnumeratorProxy MakeEmptyEnumerator()
	{ return Detail::EmptyEnumeratorProxy(); }


	namespace Detail
	{
		template < typename T >
		struct OneItemEnumerator : public virtual IEnumerator<T>
		{
		private:
			bool	_valid;
			T		_value;

		public:
			OneItemEnumerator(typename AddConstLvalueReference<T>::ValueT value)
				: _valid(true), _value(value)
			{ }

			virtual bool Valid() const
			{ return _valid; }

			virtual void Next()
			{
				STINGRAYKIT_CHECK(_valid, "Enumerator is not valid!");
				_valid = false;
			}

			virtual T Get() const
			{
				STINGRAYKIT_CHECK(_valid, "Enumerator is not valid!");
				return _value;
			}
		};

		template < typename T >
		class OneItemEnumeratorProxy
		{
		private:
			T	_item;

		public:
			explicit OneItemEnumeratorProxy(const T& item)
				: _item(item)
			{ }

			template < typename U >
			operator shared_ptr<IEnumerator<U> > () const
			{ return make_shared_ptr<OneItemEnumerator<U> >(_item); }
		};
	}


	template < typename T >
	Detail::OneItemEnumeratorProxy<T> MakeOneItemEnumerator(const T& item)
	{ return Detail::OneItemEnumeratorProxy<T>(item); }


	namespace Detail
	{
		template < typename EnumeratedT >
		class JoiningEnumerator : public virtual IEnumerator<EnumeratedT>
		{
			typedef shared_ptr<IEnumerator<EnumeratedT> > TargetEnumeratorPtr;

			TargetEnumeratorPtr _first, _second;

		public:
			JoiningEnumerator(const TargetEnumeratorPtr& first, const TargetEnumeratorPtr& second)
				:	_first(STINGRAYKIT_REQUIRE_NOT_NULL(first)),
					_second(STINGRAYKIT_REQUIRE_NOT_NULL(second))
			{ }

			virtual bool Valid() const
			{ return _first->Valid() || _second->Valid(); }

			virtual EnumeratedT Get() const
			{ return _first->Valid() ? _first->Get() : _second->Get(); }

			virtual void Next()
			{
				if (_first->Valid())
					_first->Next();
				else
					_second->Next();
			}
		};
	}

	template < typename EnumeratedT >
	shared_ptr<IEnumerator<EnumeratedT> > JoinEnumerators(const shared_ptr<IEnumerator<EnumeratedT> >& first, const shared_ptr<IEnumerator<EnumeratedT> >& second)
	{ return make_shared_ptr<Detail::JoiningEnumerator<EnumeratedT> >(first, second); }

	template < typename EnumeratedT >
	class EnumeratorJoiner
	{
		typedef shared_ptr<IEnumerator<EnumeratedT> >	EnumeratorPtr;

	private:
		EnumeratorPtr	_result;

	public:
		operator EnumeratorPtr () const { return _result; }
		EnumeratorJoiner& operator % (const EnumeratorPtr& e)
		{
			if (_result)
				_result = JoinEnumerators(_result, e);
			else
				_result = e;
			return *this;
		}
	};


	namespace Detail
	{
		template < typename T >
		struct EmptyEnumerable : public virtual IEnumerable<T>
		{
			virtual shared_ptr<IEnumerator<T> > GetEnumerator() const
			{ return make_shared_ptr<EmptyEnumerator<T> >(); }
		};

		class EmptyEnumerableProxy
		{
		public:
			template < typename U >
			operator shared_ptr<IEnumerable<U> > () const
			{ return make_shared_ptr<EmptyEnumerable<U> >(); }
		};
	}


	inline Detail::EmptyEnumerableProxy MakeEmptyEnumerable()
	{ return Detail::EmptyEnumerableProxy(); }


	namespace Detail
	{
		template < typename T >
		struct OneItemEnumerable : public virtual IEnumerable<T>
		{
		private:
			T		_value;

		public:
			OneItemEnumerable(typename AddConstLvalueReference<T>::ValueT value)
				: _value(value)
			{ }

			virtual shared_ptr<IEnumerator<T> > GetEnumerator() const
			{ return make_shared_ptr<OneItemEnumerator<T> >(_value); }
		};

		template < typename T >
		class OneItemEnumerableProxy
		{
		private:
			T	_item;

		public:
			explicit OneItemEnumerableProxy(const T& item)
				: _item(item)
			{ }

			template < typename U >
			operator shared_ptr<IEnumerable<U> > () const
			{ return make_shared_ptr<OneItemEnumerable<U> >(_item); }
		};
	}


	template < typename T >
	Detail::OneItemEnumerableProxy<T> MakeOneItemEnumerable(const T& item)
	{ return Detail::OneItemEnumerableProxy<T>(item); }


	namespace Detail
	{
		template < typename EnumeratedT >
		class JoiningEnumerable : public virtual IEnumerable<EnumeratedT>
		{
			typedef shared_ptr<IEnumerable<EnumeratedT> > TargetEnumerablePtr;

			TargetEnumerablePtr		_first;
			TargetEnumerablePtr		_second;

		public:
			JoiningEnumerable(const TargetEnumerablePtr& first, const TargetEnumerablePtr& second)
				:	_first(STINGRAYKIT_REQUIRE_NOT_NULL(first)),
					_second(STINGRAYKIT_REQUIRE_NOT_NULL(second))
			{ }

			virtual shared_ptr<IEnumerator<EnumeratedT> > GetEnumerator() const
			{ return JoinEnumerators(_first->GetEnumerator(), _second->GetEnumerator()); }
		};
	}


	template < typename EnumeratedT >
	shared_ptr<IEnumerable<EnumeratedT> > JoinEnumerables(const shared_ptr<IEnumerable<EnumeratedT> >& first, const shared_ptr<IEnumerable<EnumeratedT> >& second)
	{ return make_shared_ptr<Detail::JoiningEnumerable<EnumeratedT> >(first, second); }


	template < typename EnumeratedT >
	class EnumerableJoiner
	{
		typedef shared_ptr<IEnumerable<EnumeratedT> >	EnumerablePtr;

	private:
		EnumerablePtr	_result;

	public:
		operator EnumerablePtr () const { return Get(); }
		EnumerablePtr Get() const		{ return _result ? _result : MakeEmptyEnumerable(); }

		EnumerableJoiner& operator % (const EnumerablePtr& e)
		{
			if (_result)
				_result = JoinEnumerables(_result, e);
			else
				_result = e;
			return *this;
		}
	};


	namespace Detail
	{
		template < typename Functor_ >
		class SimpleEnumerable : public IEnumerable<typename function_info<Functor_>::RetType::ValueType::ItemType>
		{
			typedef IEnumerable<typename function_info<Functor_>::RetType::ValueType::ItemType> base;

		private:
			Functor_	_functor;

		public:
			SimpleEnumerable(const Functor_& functor) : _functor(functor)
			{ }

			virtual shared_ptr<IEnumerator<typename base::ItemType> > GetEnumerator() const
			{ return FunctorInvoker::InvokeArgs(_functor); }
		};
	}


	template < typename Functor_ >
	shared_ptr<Detail::SimpleEnumerable<Functor_> > MakeSimpleEnumerable(const Functor_& functor)
	{ return make_shared_ptr<Detail::SimpleEnumerable<Functor_> >(functor); }


	namespace Enumerable
	{

#ifdef DOXYGEN_PREPROCESSOR

		template <typename T> T Aggregate(const EnumerableOrEnumerator<T> src, const function<T (const T&, const T&)>& aggregateFunc);

		template <typename T> bool All(const EnumerableOrEnumerator<T> src, const function<bool(const T&)>& predicate);

		template <typename T> bool Any(const EnumerableOrEnumerator<T> src);
		template <typename T> bool Any(const EnumerableOrEnumerator<T> src, const function<bool(const T&)>& predicate);

		template < typename T > shared_ptr<IEnumerable<T> > Concat(const shared_ptr<IEnumerable<T> >& first, const shared_ptr<IEnumerable<T> >& second);

		template < typename CastTo, typename T > EnumerableOrEnumerator<CastTo> Cast(const EnumerableOrEnumerator<IEnumerable<T> >& src);

		template <typename T> bool Contains(const EnumerableOrEnumerator<T> src, const T& value);
		template <typename T> bool Contains(const EnumerableOrEnumerator<T> src, const T& value, const function<bool(const T&, const T&)>& equalPredicate);

		template <typename T> size_t Count(const EnumerableOrEnumerator<T> src);
		template <typename T> size_t Count(const EnumerableOrEnumerator<T> src, const function<bool(const T&)>& predicate);

		template <typename T> void ForEach(const EnumerableOrEnumerator<T> src, const function<void(const T&)>& func);

		template <typename T> optional<size_t> IndexOf(const EnumerableOrEnumerator<T> src, const T& val);
		template <typename T> optional<size_t> IndexOf(const EnumerableOrEnumerator<T> src, const function<bool(const T&)>& predicate);

		template < typename T > EnumerableOrEnumerator<T> DefaultIfEmpty(const EnumerableOrEnumerator<T>& src, const T& value = T());

		template < typename T > shared_ptr<IEnumerable<T> > Empty();

		template <typename T> T First(const EnumerableOrEnumerator<T> src);
		template <typename T> T First(const EnumerableOrEnumerator<T> src, const function<bool (const T&)>& predicate);

		template <typename T> T FirstOrDefault(const EnumerableOrEnumerator<T> src);
		template <typename T> T FirstOrDefault(const EnumerableOrEnumerator<T> src, const function<bool (const T&)>& predicate);

		template <typename T> T Last(const EnumerableOrEnumerator<T> src);
		template <typename T> T Last(const EnumerableOrEnumerator<T> src, const function<bool (const T&)>& predicate);

		template <typename T> T LastOrDefault(const EnumerableOrEnumerator<T> src);
		template <typename T> T LastOrDefault(const EnumerableOrEnumerator<T> src, const function<bool (const T&)>& predicate);

		template < typename TResult, typename T > shared_ptr<IEnumerable<TResult> > OfType(const EnumerableOrEnumerator<T>& src);

		template <typename T> T Reverse(const EnumerableOrEnumerator<T> src);

		template <typename T> T Single(const EnumerableOrEnumerator<T> src);

		template < typename T, typename FunctorType > EnumerableOrEnumerator<typename FunctorType::RetType> Transform(const EnumerableOrEnumerator<T>& src, const FunctorType& f);

		template < typename T > EnumerableOrEnumerator<T> Where(const EnumerableOrEnumerator<T>& src, const function<bool(const T&)>& predicate);

#else

#define DETAIL_ENUMERABLE_HELPER_METHODS(TemplateDecl_, RetType_, Name_) \
		TemplateDecl_ RetType_ Name_(IEnumerator<T>& enumerator); \
		TemplateDecl_ RetType_ Name_(const shared_ptr<IEnumerator<T> >& enumerator) { return Name_(*enumerator); } \
		TemplateDecl_ RetType_ Name_(const IEnumerable<T>& enumerable) { return Name_(*enumerable.GetEnumerator()); } \
		TemplateDecl_ RetType_ Name_(const shared_ptr<IEnumerable<T> >& enumerable) { STINGRAYKIT_CHECK(enumerable, NullArgumentException("enumerable")); return Name_(*enumerable); } \
		TemplateDecl_ RetType_ Name_(IEnumerator<T>& enumerator)

#define DETAIL_ENUMERABLE_HELPER_METHODS_WITH_PARAMS(TemplateDecl_, RetType_, Name_, ParamsDecl_, ParamsUsage_) \
		TemplateDecl_ RetType_ Name_(IEnumerator<T>& enumerator, ParamsDecl_); \
		TemplateDecl_ RetType_ Name_(const shared_ptr<IEnumerator<T> >& enumerator, ParamsDecl_) { return Name_(*enumerator, ParamsUsage_); } \
		TemplateDecl_ RetType_ Name_(const IEnumerable<T>& enumerable, ParamsDecl_) { return Name_(*enumerable.GetEnumerator(), ParamsUsage_); } \
		TemplateDecl_ RetType_ Name_(const shared_ptr<IEnumerable<T> >& enumerable, ParamsDecl_) { STINGRAYKIT_CHECK(enumerable, NullArgumentException("enumerable")); return Name_(*enumerable, ParamsUsage_); } \
		TemplateDecl_ RetType_ Name_(IEnumerator<T>& enumerator, ParamsDecl_)


		DETAIL_ENUMERABLE_HELPER_METHODS_WITH_PARAMS(MK_PARAM(template <typename T, typename AggregateFunc>), T, Aggregate, MK_PARAM(const AggregateFunc& aggregateFunc), MK_PARAM(aggregateFunc))
		{
			STINGRAYKIT_CHECK(enumerator.Valid(), InvalidOperationException());
			T result = enumerator.Get();
			enumerator.Next();
			for (; enumerator.Valid(); enumerator.Next())
				result = FunctorInvoker::InvokeArgs(aggregateFunc, result, enumerator.Get());
			return result;
		}


		DETAIL_ENUMERABLE_HELPER_METHODS_WITH_PARAMS(MK_PARAM(template <typename T, typename U, typename AggregateFunc>), U, Aggregate, MK_PARAM(const U& initialValue, const AggregateFunc& aggregateFunc), MK_PARAM(initialValue, aggregateFunc))
		{
			U result = initialValue;
			for (; enumerator.Valid(); enumerator.Next())
				result = FunctorInvoker::InvokeArgs(aggregateFunc, result, enumerator.Get());
			return result;
		}


		DETAIL_ENUMERABLE_HELPER_METHODS_WITH_PARAMS(MK_PARAM(template <typename T, typename PredicateFunc>), bool, All, MK_PARAM(const PredicateFunc& predicate), MK_PARAM(predicate))
		{
			for (; enumerator.Valid(); enumerator.Next())
				if (!FunctorInvoker::InvokeArgs(predicate, enumerator.Get()))
					return false;
			return true;
		}


		DETAIL_ENUMERABLE_HELPER_METHODS(MK_PARAM(template <typename T>), bool, Any)
		{ return enumerator.Valid(); }

		DETAIL_ENUMERABLE_HELPER_METHODS_WITH_PARAMS(MK_PARAM(template <typename T, typename PredicateFunc>), bool, Any, MK_PARAM(const PredicateFunc& predicate), MK_PARAM(predicate))
		{
			for (; enumerator.Valid(); enumerator.Next())
				if (FunctorInvoker::InvokeArgs(predicate, enumerator.Get()))
					return true;
			return false;
		}


		template < typename T >
		shared_ptr<IEnumerable<T> > Concat(const shared_ptr<IEnumerable<T> >& first, const shared_ptr<IEnumerable<T> >& second)
		{ return make_shared_ptr<Detail::JoiningEnumerable<T> >(first, second); }


		template < typename CastTo, typename T >
		shared_ptr<IEnumerable<CastTo> > Cast(const shared_ptr<IEnumerable<T> >& enumerable)
		{ return Detail::EnumerableCaster<T>(enumerable); }


		DETAIL_ENUMERABLE_HELPER_METHODS_WITH_PARAMS(MK_PARAM(template <typename T, typename EqualPredicateFunc>), bool, Contains, MK_PARAM(const T& value, const EqualPredicateFunc& equalPredicate), MK_PARAM(value, equalPredicate))
		{
			for (; enumerator.Valid(); enumerator.Next())
				if (FunctorInvoker::InvokeArgs(equalPredicate, enumerator.Get(), value))
					return true;
			return false;
		}


		DETAIL_ENUMERABLE_HELPER_METHODS_WITH_PARAMS(MK_PARAM(template <typename T>), bool, Contains, MK_PARAM(const T& value), MK_PARAM(value))
		{ return Contains(enumerator, value, std::equal_to<T>()); }


		DETAIL_ENUMERABLE_HELPER_METHODS(MK_PARAM(template <typename T>), size_t, Count)
		{
			size_t result = 0;
			for (; enumerator.Valid(); enumerator.Next())
				++result;
			return result;
		}

		DETAIL_ENUMERABLE_HELPER_METHODS_WITH_PARAMS(MK_PARAM(template <typename T, typename PredicateFunc>), size_t, Count, MK_PARAM(const PredicateFunc& predicate), MK_PARAM(predicate))
		{
			size_t result = 0;
			for (; enumerator.Valid(); enumerator.Next())
				if (FunctorInvoker::InvokeArgs(predicate, enumerator.Get()))
					++result;
			return result;
		}


		DETAIL_ENUMERABLE_HELPER_METHODS_WITH_PARAMS(MK_PARAM(template <typename T, typename Func>), void, ForEach, MK_PARAM(const Func& func), MK_PARAM(func))
		{
			for (; enumerator.Valid(); enumerator.Next())
				FunctorInvoker::InvokeArgs(func, enumerator.Get());
		}


		DETAIL_ENUMERABLE_HELPER_METHODS_WITH_PARAMS(MK_PARAM(template <typename T, typename PredicateFunc>), optional<size_t>, IndexOf, MK_PARAM(const PredicateFunc& predicate), MK_PARAM(predicate))
		{
			size_t result = 0;
			for (; enumerator.Valid(); enumerator.Next())
			{
				if (FunctorInvoker::InvokeArgs(predicate, enumerator.Get()))
					return result;
				++result;
			}
			return null;
		}

		DETAIL_ENUMERABLE_HELPER_METHODS_WITH_PARAMS(MK_PARAM(template <typename T>), optional<size_t>, IndexOf, MK_PARAM(const T& val), MK_PARAM(val))
		{ return IndexOf(enumerator, Bind(std::equal_to<T>(), val, _1)); }


		DETAIL_ENUMERABLE_HELPER_METHODS_WITH_PARAMS(MK_PARAM(template <typename T>), T, ElementAt, MK_PARAM(size_t index), MK_PARAM(index))
		{
			size_t current = 0;
			for (; enumerator.Valid(); enumerator.Next(), ++current)
				if (index == current)
					return enumerator.Get();
			STINGRAYKIT_THROW(IndexOutOfRangeException(index, current));
		}

		DETAIL_ENUMERABLE_HELPER_METHODS_WITH_PARAMS(MK_PARAM(template <typename T>), T, ElementAtOrDefault, MK_PARAM(size_t index), MK_PARAM(index))
		{
			size_t current = 0;
			for (; enumerator.Valid(); enumerator.Next(), ++current)
				if (index == current)
					return enumerator.Get();
			return T();
		}


		template < typename T >
		shared_ptr<IEnumerable<T> > DefaultIfEmpty(const shared_ptr<IEnumerable<T> >& src, const T& value = T())
		{ return Any(src) ? src : MakeOneItemEnumerable(value); }


		template < typename T >
		shared_ptr<IEnumerable<T> > Empty()
		{ return MakeEmptyEnumerable(); }


		DETAIL_ENUMERABLE_HELPER_METHODS(MK_PARAM(template <typename T>), T, First)
		{
			STINGRAYKIT_CHECK(enumerator.Valid(), InvalidOperationException());
			return enumerator.Get();
		}

		DETAIL_ENUMERABLE_HELPER_METHODS_WITH_PARAMS(MK_PARAM(template <typename T, typename PredicateFunc>), T, First, MK_PARAM(const PredicateFunc& predicate), MK_PARAM(predicate))
		{
			while (enumerator.Valid() && !FunctorInvoker::InvokeArgs(predicate, enumerator.Get()))
				enumerator.Next();
			STINGRAYKIT_CHECK(enumerator.Valid(), InvalidOperationException());
			return enumerator.Get();
		}


		DETAIL_ENUMERABLE_HELPER_METHODS(MK_PARAM(template <typename T>), T, FirstOrDefault)
		{
			return enumerator.Valid() ? enumerator.Get() : T();
		}

		DETAIL_ENUMERABLE_HELPER_METHODS_WITH_PARAMS(MK_PARAM(template <typename T, typename PredicateFunc>), T, FirstOrDefault, MK_PARAM(const PredicateFunc& predicate), MK_PARAM(predicate))
		{
			for (; enumerator.Valid(); enumerator.Next())
			{
				const T result = enumerator.Get();
				if (FunctorInvoker::InvokeArgs(predicate, result))
					return result;
			}
			return T();
		}


		DETAIL_ENUMERABLE_HELPER_METHODS(MK_PARAM(template <typename T>), T, Last)
		{
			optional<T> result;
			for (; enumerator.Valid(); enumerator.Next())
				result = enumerator.Get();
			STINGRAYKIT_CHECK(result, InvalidOperationException());
			return *result;
		}

		DETAIL_ENUMERABLE_HELPER_METHODS_WITH_PARAMS(MK_PARAM(template <typename T, typename PredicateFunc>), T, Last, MK_PARAM(const PredicateFunc& predicate), MK_PARAM(predicate))
		{
			optional<T> result;
			for (; enumerator.Valid(); enumerator.Next())
			{
				const T value = enumerator.Get();
				if (FunctorInvoker::InvokeArgs(predicate, value))
					result = value;
			}
			STINGRAYKIT_CHECK(result, InvalidOperationException());
			return *result;
		}


		DETAIL_ENUMERABLE_HELPER_METHODS(MK_PARAM(template <typename T>), T, LastOrDefault)
		{
			optional<T> result;
			for (; enumerator.Valid(); enumerator.Next())
				result = enumerator.Get();
			return result ? *result : T();
		}

		DETAIL_ENUMERABLE_HELPER_METHODS_WITH_PARAMS(MK_PARAM(template <typename T, typename PredicateFunc>), T, LastOrDefault, MK_PARAM(const PredicateFunc& predicate), MK_PARAM(predicate))
		{
			optional<T> result;
			for (; enumerator.Valid(); enumerator.Next())
			{
				const T value = enumerator.Get();
				if (FunctorInvoker::InvokeArgs(predicate, value))
					result = value;
			}
			return result ? *result : T();
		}


		namespace Detail
		{
			template < typename Dst_, typename SrcEnumerator_ >
			class EnumeratorOfType : public IEnumerator<Dst_>
			{
				typedef RefStorage<Dst_> Storage;

			private:
				SrcEnumerator_              _srcEnumerator;
				typename Storage::ValueType _dst;

			public:
				EnumeratorOfType(const SrcEnumerator_& srcEnumerator) : _srcEnumerator(srcEnumerator)
				{ FindNext(); }

				virtual bool Valid() const
				{ return _srcEnumerator->Valid(); }

				virtual Dst_ Get() const
				{
					STINGRAYKIT_CHECK(_srcEnumerator->Valid(), "Enumerator is not valid!");
					return Storage::Unwrap(_dst);
				}

				virtual void Next()
				{
					_srcEnumerator->Next();
					FindNext();
				}

			private:
				void FindNext()
				{
					for ( ; _srcEnumerator->Valid(); _srcEnumerator->Next())
					{
						_dst = DynamicCast<typename Storage::ValueType>(Storage::Wrap(_srcEnumerator->Get()));
						if (_dst)
							return;
					}
				}
			};
		}


		template < typename TResult, typename SrcEnumerator >
		shared_ptr<IEnumerator<TResult> > OfType(const shared_ptr<SrcEnumerator>& enumerator, typename EnableIf<IsEnumerator<SrcEnumerator>::Value, int>::ValueT dummy = 0)
		{ return make_shared_ptr<Detail::EnumeratorOfType<TResult, shared_ptr<SrcEnumerator> > >(enumerator); }


		template < typename TResult, typename SrcEnumerable >
		shared_ptr<IEnumerable<TResult> > OfType(const shared_ptr<SrcEnumerable>& enumerable, typename EnableIf<IsEnumerable<SrcEnumerable>::Value, int>::ValueT dummy = 0)
		{ return MakeSimpleEnumerable(Bind(MakeShared<Detail::EnumeratorOfType<TResult, shared_ptr<IEnumerator<typename SrcEnumerable::ItemType> > > >(), Bind(&SrcEnumerable::GetEnumerator, enumerable))); }


		template < typename CollectionType >
		shared_ptr<IEnumerator<typename CollectionType::ItemType> > Reverse(const shared_ptr<CollectionType>& enumerator, typename EnableIf<IsEnumerator<CollectionType>::Value, int>::ValueT dummy = 0)
		{
			const shared_ptr<std::vector<typename CollectionType::ItemType> > result = make_shared_ptr<std::vector<typename CollectionType::ItemType> >();
			for (; enumerator.Valid(); enumerator.Next())
				result->push_back(enumerator.Get());
			return EnumeratorFromStlIterators(result->rbegin(), result->rend(), result);
		}

		template < typename CollectionType >
		shared_ptr<IEnumerable<typename CollectionType::ItemType> > Reverse(const shared_ptr<CollectionType>& enumerable, typename EnableIf<IsEnumerable<CollectionType>::Value && !IsReversableEnumerable<CollectionType>::Value, int>::ValueT dummy = 0)
		{
			const shared_ptr<std::vector<typename CollectionType::ItemType> > result = make_shared_ptr<std::vector<typename CollectionType::ItemType> >();
			for (shared_ptr<IEnumerator<typename CollectionType::ItemType> > enumerator = enumerable->GetEnumerator(); enumerator->Valid(); enumerator->Next())
				result->push_back(enumerator->Get());
			return EnumerableFromStlIterators(result->rbegin(), result->rend(), result);
		}

		template < typename CollectionType >
		shared_ptr<IEnumerable<typename CollectionType::ItemType> > Reverse(const shared_ptr<CollectionType>& enumerable, typename EnableIf<IsEnumerable<CollectionType>::Value && IsReversableEnumerable<CollectionType>::Value, int>::ValueT dummy = 0)
		{ return enumerable->Reverse(); }


		DETAIL_ENUMERABLE_HELPER_METHODS(MK_PARAM(template <typename T>), T, Single)
		{
			STINGRAYKIT_CHECK(enumerator.Valid(), InvalidOperationException());
			T result = enumerator.Get();
			enumerator.Next();
			STINGRAYKIT_CHECK(!enumerator.Valid(), InvalidOperationException());
			return result;
		}


		namespace Detail
		{
			template < typename SrcType, typename FunctorType >
			class EnumeratorTransformer : public IEnumerator<typename function_info<FunctorType>::RetType>
			{
				typedef IEnumerator<typename function_info<FunctorType>::RetType>	base;
				typedef shared_ptr<IEnumerator<SrcType> >							SrcEnumeratorPtr;

			private:
				SrcEnumeratorPtr	_src;
				FunctorType			_functor;

			public:
				EnumeratorTransformer(const SrcEnumeratorPtr& src, const FunctorType& functor)
					: _src(src), _functor(functor)
				{ }

				virtual bool Valid() const					{ return _src->Valid(); }
				virtual typename base::ItemType Get() const	{ return FunctorInvoker::InvokeArgs(_functor, _src->Get()); }
				virtual void Next()							{ _src->Next(); }
			};


			template < typename SrcType, typename FunctorType >
			class EnumerableTransformer : public IEnumerable<typename function_info<FunctorType>::RetType>
			{
				typedef IEnumerable<typename function_info<FunctorType>::RetType>	base;
				typedef shared_ptr<IEnumerable<SrcType> >							SrcEnumerablePtr;

			private:
				SrcEnumerablePtr	_src;
				FunctorType			_functor;

			public:
				EnumerableTransformer(const SrcEnumerablePtr& src, const FunctorType& functor)
					: _src(src), _functor(functor)
				{ }

				virtual shared_ptr<IEnumerator<typename base::ItemType> > GetEnumerator() const
				{ return make_shared_ptr<EnumeratorTransformer<SrcType, FunctorType> >(_src->GetEnumerator(), _functor); }
			};
		}


		template < typename SrcEnumerator, typename FunctorType >
		shared_ptr<IEnumerator<typename function_info<FunctorType>::RetType> > Transform(const shared_ptr<SrcEnumerator>& enumerator, const FunctorType& functor, typename EnableIf<IsEnumerator<SrcEnumerator>::Value, int>::ValueT dummy = 0)
		{ return make_shared_ptr<Detail::EnumeratorTransformer<typename SrcEnumerator::ItemType, FunctorType> >(enumerator, functor); }


		template < typename SrcEnumerable, typename FunctorType >
		shared_ptr<IEnumerable<typename function_info<FunctorType>::RetType> > Transform(const shared_ptr<SrcEnumerable>& enumerable, const FunctorType& functor, typename EnableIf<IsEnumerable<SrcEnumerable>::Value, int>::ValueT dummy = 0)
		{ return make_shared_ptr<Detail::EnumerableTransformer<typename SrcEnumerable::ItemType, FunctorType> >(enumerable, functor); }


		template < typename CollectionType, typename PredicateFunc >
		shared_ptr<IEnumerator<typename CollectionType::ItemType> > Where(const shared_ptr<CollectionType>& enumerator, const PredicateFunc& predicate, typename EnableIf<IsEnumerator<CollectionType>::Value, int>::ValueT dummy = 0)
		{
			typedef typename CollectionType::ItemType T;
			return WrapEnumerator(enumerator, &stingray::implicit_cast<T>, predicate);
		}

		template < typename CollectionType, typename PredicateFunc >
		shared_ptr<IEnumerable<typename CollectionType::ItemType> > Where(const shared_ptr<CollectionType>& enumerable, const PredicateFunc& predicate, typename EnableIf<IsEnumerable<CollectionType>::Value, int>::ValueT dummy = 0)
		{
			typedef typename CollectionType::ItemType T;
			return WrapEnumerable(enumerable, &stingray::implicit_cast<T>, predicate);
		}


		namespace Detail
		{
			template < typename ItemType >
			shared_ptr<IEnumerator<ItemType>> SkipEnumerator(const shared_ptr<IEnumerator<ItemType>>& src, size_t count)
			{
				for (size_t index = 0; index < count && src->Valid(); ++index)
					src->Next();

				return src;
			}
		}


		template < typename SrcEnumerator >
		shared_ptr<IEnumerator<typename SrcEnumerator::ItemType>> Skip(const shared_ptr<SrcEnumerator>& enumerator, size_t count, typename EnableIf<IsEnumerator<SrcEnumerator>::Value, int>::ValueT dummy = 0)
		{ return Detail::SkipEnumerator(enumerator, count); }


		template < typename SrcEnumerable >
		shared_ptr<IEnumerable<typename SrcEnumerable::ItemType>> Skip(const shared_ptr<SrcEnumerable>& enumerable, size_t count, typename EnableIf<IsEnumerable<SrcEnumerable>::Value, int>::ValueT dummy = 0)
		{ return MakeSimpleEnumerable(Bind(&Detail::SkipEnumerator<typename SrcEnumerable::ItemType>, Bind(&SrcEnumerable::GetEnumerator, enumerable), count)); }


		namespace Detail
		{
			template < typename ItemType >
			class EnumeratorTaker : public IEnumerator<ItemType>
			{
				typedef shared_ptr<IEnumerator<ItemType>>							SrcEnumeratorPtr;

			private:
				SrcEnumeratorPtr	_src;
				size_t				_index;
				size_t				_count;

			public:
				EnumeratorTaker(const SrcEnumeratorPtr& src, size_t count)
					: _src(src), _index(0), _count(count)
				{ }

				virtual bool Valid() const
				{ return _src->Valid() && _index < _count; }

				virtual ItemType Get() const
				{
					STINGRAYKIT_CHECK(Valid(), "Enumerator is not valid!");
					return _src->Get();
				}

				virtual void Next()
				{
					STINGRAYKIT_CHECK(Valid(), "Enumerator is not valid!");
					_src->Next();
					++_index;
				}
			};
		}


		template < typename SrcEnumerator >
		shared_ptr<IEnumerator<typename SrcEnumerator::ItemType>> Take(const shared_ptr<SrcEnumerator>& enumerator, size_t count, typename EnableIf<IsEnumerator<SrcEnumerator>::Value, int>::ValueT dummy = 0)
		{ return make_shared_ptr<Detail::EnumeratorTaker<typename SrcEnumerator::ItemType>>(enumerator, count); }


		template < typename SrcEnumerable >
		shared_ptr<IEnumerable<typename SrcEnumerable::ItemType>> Take(const shared_ptr<SrcEnumerable>& enumerable, size_t count, typename EnableIf<IsEnumerable<SrcEnumerable>::Value, int>::ValueT dummy = 0)
		{ return MakeSimpleEnumerable(Bind(MakeShared<Detail::EnumeratorTaker<typename SrcEnumerable::ItemType>>(), Bind(&SrcEnumerable::GetEnumerator, enumerable), count)); }


		template < typename T, typename PredicateFunc >
		bool SequenceEqual(const shared_ptr<IEnumerator<T> >& first, const shared_ptr<IEnumerator<T> >& second, const PredicateFunc& equalPredicate)
		{
			for (; first->Valid() && second->Valid(); first->Next(), second->Next())
				if (!FunctorInvoker::InvokeArgs(equalPredicate, first->Get(), second->Get()))
					return false;
			return !first->Valid() && !second->Valid();
		}


		template < typename T >
		bool SequenceEqual(const shared_ptr<IEnumerator<T> >& first, const shared_ptr<IEnumerator<T> >& second)
		{ return SequenceEqual(first, second, std::equal_to<T>()); }


		template < typename First, typename Second, typename PredicateFunc >
		bool SequenceEqual(const shared_ptr<First>& first, const shared_ptr<Second>& second, const PredicateFunc& equalPredicate, typename EnableIf<!IsEnumerator<First>::Value || !IsEnumerator<Second>::Value, int>::ValueT dummy = 0)
		{ return SequenceEqual(ToEnumerator(first), ToEnumerator(second), equalPredicate); }


		template < typename First, typename Second >
		bool SequenceEqual(const shared_ptr<First>& first, const shared_ptr<Second>& second, typename EnableIf<!IsEnumerator<First>::Value || !IsEnumerator<Second>::Value, int>::ValueT dummy = 0)
		{ return SequenceEqual(ToEnumerator(first), ToEnumerator(second)); }


		template < typename CompareFunc >
		class SequenceCmp : public function_info<int, UnspecifiedParamTypes>
		{
		private:
			CompareFunc	_compareFunc;

		public:
			SequenceCmp(const CompareFunc& compareFunc = CompareFunc()) : _compareFunc(compareFunc)
			{ }

			template < typename T >
			int operator () (const shared_ptr<T>& first, const shared_ptr<T>& second) const
			{
				shared_ptr<IEnumerator<typename T::ItemType> > l(first->GetEnumerator()), r(second->GetEnumerator());
				for (; l->Valid(); l->Next(), r->Next())
				{
					if (!r->Valid())
						return 1;

					const int itemResult = _compareFunc(l->Get(), r->Get());
					if (itemResult != 0)
						return itemResult;
				}
				return r->Valid() ? -1 : 0;
			}
		};


		inline SequenceCmp<ComparableCmp> MakeSequenceCmp()
		{ return SequenceCmp<ComparableCmp>(); }


		template < typename CompareFunc >
		SequenceCmp<CompareFunc> MakeSequenceCmp(const CompareFunc& compareFunc)
		{ return SequenceCmp<CompareFunc>(compareFunc); }

#undef DETAIL_ENUMERABLE_HELPER_METHODS_WITH_PARAMS
#undef DETAIL_ENUMERABLE_HELPER_METHODS

#endif

	}


	template < typename Enumerable_ >
	struct FirstTransformerImpl<shared_ptr<Enumerable_>, typename EnableIf<IsEnumerable<Enumerable_>::Value, void>::ValueT>
	{
		typedef typename Enumerable_::ItemType ValueT;

		static ValueT Do(const shared_ptr<Enumerable_>& enumerable, const FirstTransformer& action)
		{ return Enumerable::First(enumerable); }
	};


	template < typename Enumerable_ >
	struct FirstOrDefaultTransformerImpl<shared_ptr<Enumerable_>, typename EnableIf<IsEnumerable<Enumerable_>::Value, void>::ValueT>
	{
		typedef typename Enumerable_::ItemType ValueT;

		static ValueT Do(const shared_ptr<Enumerable_>& enumerable, const FirstOrDefaultTransformer& action)
		{ return Enumerable::FirstOrDefault(enumerable); }
	};


	template < typename Enumerable_, typename Predicate_ >
	struct FilterTransformerImpl<shared_ptr<Enumerable_>, Predicate_, typename EnableIf<IsEnumerable<Enumerable_>::Value, void>::ValueT>
	{
		typedef typename Enumerable_::ItemType ItemType;
		typedef shared_ptr<IEnumerable<ItemType> > ValueT;

		static ValueT Do(const shared_ptr<Enumerable_>& enumerable, const FilterTransformer<Predicate_>& action)
		{ return Enumerable::Where(enumerable, action.GetPredicate()); }
	};


	template < typename Enumerable_ >
	struct ReverseTransformerImpl<shared_ptr<Enumerable_>, typename EnableIf<IsEnumerable<Enumerable_>::Value, void>::ValueT>
	{
		typedef typename Enumerable_::ItemType ItemType;
		typedef shared_ptr<IEnumerable<ItemType> > ValueT;

		static ValueT Do(const shared_ptr<Enumerable_>& enumerable, const ReverseTransformer& action)
		{ return Enumerable::Reverse(enumerable); }
	};


	template < typename Enumerable_, typename Functor_ >
	struct TransformTransformerImpl<shared_ptr<Enumerable_>, Functor_, typename EnableIf<IsEnumerable<Enumerable_>::Value, void>::ValueT>
	{
		typedef typename function_info<Functor_>::RetType ItemType;
		typedef shared_ptr<IEnumerable<ItemType> > ValueT;

		static ValueT Do(const shared_ptr<Enumerable_>& enumerable, const TransformTransformer<Functor_>& action)
		{ return Enumerable::Transform(enumerable, action.GetFunctor()); }
	};


	template < typename Enumerable_, typename Dst_ >
	struct CastTransformerImpl<shared_ptr<Enumerable_>, Dst_, typename EnableIf<IsEnumerable<Enumerable_>::Value, void>::ValueT>
	{
		typedef shared_ptr<IEnumerable<Dst_> > ValueT;

		static ValueT Do(const shared_ptr<Enumerable_>& enumerable, const CastTransformer<Dst_>& action)
		{ return Enumerable::Cast<Dst_>(enumerable); }
	};


	template < typename Enumerable_, typename Dst_ >
	struct OfTypeTransformerImpl<shared_ptr<Enumerable_>, Dst_, typename EnableIf<IsEnumerable<Enumerable_>::Value, void>::ValueT>
	{
		typedef shared_ptr<IEnumerable<Dst_> > ValueT;

		static ValueT Do(const shared_ptr<Enumerable_>& enumerable, const OfTypeTransformer<Dst_>& action)
		{ return Enumerable::OfType<Dst_>(enumerable); }
	};


	template < typename Enumerable_ >
	struct AnyTransformerImpl<shared_ptr<Enumerable_>, typename EnableIf<IsEnumerable<Enumerable_>::Value, void>::ValueT>
	{
		typedef bool ValueT;

		static ValueT Do(const shared_ptr<Enumerable_>& enumerable, const AnyTransformer& action)
		{ return Enumerable::Any(enumerable); }
	};


	template < typename Enumerable_ >
	struct CountTransformerImpl<shared_ptr<Enumerable_>, typename EnableIf<IsEnumerable<Enumerable_>::Value, void>::ValueT>
	{
		typedef size_t ValueT;

		static ValueT Do(const shared_ptr<Enumerable_>& enumerable, const CountTransformer& action)
		{ return Enumerable::Count(enumerable); }
	};


	template < typename Enumerable_ >
	struct DropTransformerImpl<shared_ptr<Enumerable_>, typename EnableIf<IsEnumerable<Enumerable_>::Value, void>::ValueT>
	{
		typedef typename Enumerable_::ItemType ItemType;
		typedef shared_ptr<IEnumerable<ItemType>> ValueT;

		static ValueT Do(const shared_ptr<Enumerable_>& enumerable, const DropTransformer& action)
		{ return Enumerable::Skip(enumerable, action.GetCount()); }
	};


	template < typename Enumerable_ >
	struct TakeTransformerImpl<shared_ptr<Enumerable_>, typename EnableIf<IsEnumerable<Enumerable_>::Value, void>::ValueT>
	{
		typedef typename Enumerable_::ItemType ItemType;
		typedef shared_ptr<IEnumerable<ItemType>> ValueT;

		static ValueT Do(const shared_ptr<Enumerable_>& enumerable, const TakeTransformer& action)
		{ return Enumerable::Take(enumerable, action.GetCount()); }
	};

	/** @} */

}

#endif
