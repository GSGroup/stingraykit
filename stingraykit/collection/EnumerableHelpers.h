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
			bool Valid() const override		{ return false; }
			T Get() const override			{ STINGRAYKIT_THROW("Enumerator is not valid!"); }
			void Next() override			{ STINGRAYKIT_THROW("Enumerator is not valid!"); }
		};

		class EmptyEnumeratorProxy
		{
		public:
			template < typename U >
			operator shared_ptr<IEnumerator<U>> () const
			{ return make_shared_ptr<EmptyEnumerator<U>>(); }
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

			bool Valid() const override
			{ return _valid; }

			void Next() override
			{
				STINGRAYKIT_CHECK(_valid, "Enumerator is not valid!");
				_valid = false;
			}

			T Get() const override
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
			operator shared_ptr<IEnumerator<U>> () const
			{ return make_shared_ptr<OneItemEnumerator<U>>(_item); }
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
			using TargetEnumeratorPtr = shared_ptr<IEnumerator<EnumeratedT>>;

		private:
			TargetEnumeratorPtr		_first;
			TargetEnumeratorPtr		_second;

		public:
			JoiningEnumerator(const TargetEnumeratorPtr& first, const TargetEnumeratorPtr& second)
			{
				if (STINGRAYKIT_REQUIRE_NOT_NULL(first)->Valid())
				{
					_first = first;
					_second = STINGRAYKIT_REQUIRE_NOT_NULL(second);
				}
				else
					_first = STINGRAYKIT_REQUIRE_NOT_NULL(second);
			}

			bool Valid() const override
			{ return _first && _first->Valid(); }

			EnumeratedT Get() const override
			{
				STINGRAYKIT_CHECK(_first, "Enumerator is not valid!");
				return _first->Get();
			}

			void Next() override
			{
				STINGRAYKIT_CHECK(_first, "Enumerator is not valid!");
				_first->Next();
				if (_first->Valid())
					return;

				_first.swap(_second);
				_second.reset();
			}
		};
	}


	template < typename EnumeratedT >
	shared_ptr<IEnumerator<EnumeratedT>> JoinEnumerators(const shared_ptr<IEnumerator<EnumeratedT>>& first, const shared_ptr<IEnumerator<EnumeratedT>>& second)
	{ return make_shared_ptr<Detail::JoiningEnumerator<EnumeratedT>>(first, second); }


	template < typename EnumeratedT >
	class EnumeratorJoiner
	{
		using EnumeratorPtr = shared_ptr<IEnumerator<EnumeratedT>>;

	private:
		EnumeratorPtr	_result;

	public:
		operator EnumeratorPtr () const { return Get(); }
		EnumeratorPtr Get() const		{ return _result ? _result : MakeEmptyEnumerator(); }

		EnumeratorJoiner& operator % (const EnumeratorPtr& enumerator)
		{
			if (_result)
				_result = JoinEnumerators(_result, enumerator);
			else
				_result = STINGRAYKIT_REQUIRE_NOT_NULL(enumerator);
			return *this;
		}
	};


	namespace Detail
	{
		template < typename T >
		struct EmptyEnumerable : public virtual IEnumerable<T>
		{
			shared_ptr<IEnumerator<T>> GetEnumerator() const override
			{ return make_shared_ptr<EmptyEnumerator<T>>(); }
		};

		class EmptyEnumerableProxy
		{
		public:
			template < typename U >
			operator shared_ptr<IEnumerable<U>> () const
			{ return make_shared_ptr<EmptyEnumerable<U>>(); }
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

			shared_ptr<IEnumerator<T>> GetEnumerator() const override
			{ return make_shared_ptr<OneItemEnumerator<T>>(_value); }
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
			operator shared_ptr<IEnumerable<U>> () const
			{ return make_shared_ptr<OneItemEnumerable<U>>(_item); }
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
			using TargetEnumerablePtr = shared_ptr<IEnumerable<EnumeratedT>>;

		private:
			TargetEnumerablePtr		_first;
			TargetEnumerablePtr		_second;

		public:
			JoiningEnumerable(const TargetEnumerablePtr& first, const TargetEnumerablePtr& second)
				:	_first(STINGRAYKIT_REQUIRE_NOT_NULL(first)),
					_second(STINGRAYKIT_REQUIRE_NOT_NULL(second))
			{ }

			shared_ptr<IEnumerator<EnumeratedT>> GetEnumerator() const override
			{ return JoinEnumerators(_first->GetEnumerator(), _second->GetEnumerator()); }
		};
	}


	template < typename EnumeratedT >
	shared_ptr<IEnumerable<EnumeratedT>> JoinEnumerables(const shared_ptr<IEnumerable<EnumeratedT>>& first, const shared_ptr<IEnumerable<EnumeratedT>>& second)
	{ return make_shared_ptr<Detail::JoiningEnumerable<EnumeratedT>>(first, second); }


	template < typename EnumeratedT >
	class EnumerableJoiner
	{
		using EnumerablePtr = shared_ptr<IEnumerable<EnumeratedT>>;

	private:
		EnumerablePtr	_result;

	public:
		operator EnumerablePtr () const { return Get(); }
		EnumerablePtr Get() const		{ return _result ? _result : MakeEmptyEnumerable(); }

		EnumerableJoiner& operator % (const EnumerablePtr& enumerable)
		{
			if (_result)
				_result = JoinEnumerables(_result, enumerable);
			else
				_result = STINGRAYKIT_REQUIRE_NOT_NULL(enumerable);
			return *this;
		}
	};


	namespace Detail
	{
		template < typename Functor_ >
		class SimpleEnumerable : public virtual IEnumerable<typename function_info<Functor_>::RetType::ValueType::ItemType>
		{
			using base = IEnumerable<typename function_info<Functor_>::RetType::ValueType::ItemType>;

		private:
			Functor_	_functor;

		public:
			SimpleEnumerable(const Functor_& functor) : _functor(functor)
			{ }

			shared_ptr<IEnumerator<typename base::ItemType>> GetEnumerator() const override
			{ return FunctorInvoker::InvokeArgs(_functor); }
		};
	}


	template < typename Functor_ >
	shared_ptr<Detail::SimpleEnumerable<Functor_>> MakeSimpleEnumerable(const Functor_& functor)
	{ return make_shared_ptr<Detail::SimpleEnumerable<Functor_>>(functor); }


	namespace Enumerable
	{

#ifdef DOXYGEN_PREPROCESSOR

		template <typename T> T Aggregate(const EnumerableOrEnumerator<T> src, const function<T (const T&, const T&)>& aggregateFunc);
		template <typename T> T Aggregate(const EnumerableOrEnumerator<T> src, const T& initialValue, const function<T (const T&, const T&)>& aggregateFunc);

		template <typename T> bool All(const EnumerableOrEnumerator<T> src, const function<bool(const T&)>& predicate);

		template <typename T> bool Any(const EnumerableOrEnumerator<T> src);
		template <typename T> bool Any(const EnumerableOrEnumerator<T> src, const function<bool(const T&)>& predicate);

		template < typename T > shared_ptr<IEnumerable<T>> Concat(const shared_ptr<IEnumerable<T>>& first, const shared_ptr<IEnumerable<T>>& second);

		template < typename CastTo, typename T > EnumerableOrEnumerator<CastTo> Cast(const EnumerableOrEnumerator<IEnumerable<T>>& src);

		template <typename T> bool Contains(const EnumerableOrEnumerator<T> src, const T& value);
		template <typename T> bool Contains(const EnumerableOrEnumerator<T> src, const T& value, const function<bool(const T&, const T&)>& equalPredicate);

		template <typename T> size_t Count(const EnumerableOrEnumerator<T> src);
		template <typename T> size_t Count(const EnumerableOrEnumerator<T> src, const function<bool(const T&)>& predicate);

		template <typename T> void ForEach(const EnumerableOrEnumerator<T> src, const function<void(const T&)>& func);

		template <typename T> optional<size_t> IndexOf(const EnumerableOrEnumerator<T> src, const T& val);
		template <typename T> optional<size_t> IndexOf(const EnumerableOrEnumerator<T> src, const function<bool(const T&)>& predicate);

		template <typename T> T ElementAt(const EnumerableOrEnumerator<T> src, size_t index);
		template <typename T> T ElementAtOrDefault(const EnumerableOrEnumerator<T> src, size_t index);

		template < typename T > EnumerableOrEnumerator<T> DefaultIfEmpty(const EnumerableOrEnumerator<T>& src, const T& value = T());

		template < typename T > shared_ptr<IEnumerable<T>> Empty();

		template <typename T> T First(const EnumerableOrEnumerator<T> src);
		template <typename T> T First(const EnumerableOrEnumerator<T> src, const function<bool (const T&)>& predicate);

		template <typename T> T FirstOrDefault(const EnumerableOrEnumerator<T> src);
		template <typename T> T FirstOrDefault(const EnumerableOrEnumerator<T> src, const function<bool (const T&)>& predicate);

		template <typename T> T Last(const EnumerableOrEnumerator<T> src);
		template <typename T> T Last(const EnumerableOrEnumerator<T> src, const function<bool (const T&)>& predicate);

		template <typename T> T LastOrDefault(const EnumerableOrEnumerator<T> src);
		template <typename T> T LastOrDefault(const EnumerableOrEnumerator<T> src, const function<bool (const T&)>& predicate);

		template < typename TResult, typename T > shared_ptr<IEnumerable<TResult>> OfType(const EnumerableOrEnumerator<T>& src);

		template <typename T> T Reverse(const EnumerableOrEnumerator<T> src);

		template <typename T> T Single(const EnumerableOrEnumerator<T> src);

		template < typename T, typename FunctorType > EnumerableOrEnumerator<typename FunctorType::RetType> Transform(const EnumerableOrEnumerator<T>& src, const FunctorType& f);

		template < typename T > EnumerableOrEnumerator<T> Where(const EnumerableOrEnumerator<T>& src, const function<bool(const T&)>& predicate);

		template < typename T > EnumerableOrEnumerator<T> Skip(const EnumerableOrEnumerator<T>& src, size_t count);

		template < typename T > EnumerableOrEnumerator<T> Take(const EnumerableOrEnumerator<T>& src, size_t count);

		template < typename TResult, typename T > shared_ptr<IEnumerable<TResult>> Flatten(const EnumerableOrEnumerator<T>& enumerableOfEnumerables);

		template < typename T > bool SequenceEqual(const EnumerableOrEnumerator<T>& first, const EnumerableOrEnumerator<T>& second);
		template < typename T > bool SequenceEqual(const EnumerableOrEnumerator<T>& first, const EnumerableOrEnumerator<T>& second, const function<bool(const T&, const T&)>& comparer);

#else

#define DETAIL_ENUMERABLE_HELPER_METHODS(TemplateDecl_, RetType_, Name_, ParamsDecl_, ParamsUsage_, ...) \
		template < typename ItemType STINGRAYKIT_COMMA_IF(STINGRAYKIT_NARGS(TemplateDecl_)) TemplateDecl_ > \
		RetType_ Name_(IEnumerator<ItemType>& enumerator STINGRAYKIT_COMMA_IF(STINGRAYKIT_NARGS(ParamsDecl_)) ParamsDecl_) \
		{ __VA_ARGS__; } \
		template < typename ItemType STINGRAYKIT_COMMA_IF(STINGRAYKIT_NARGS(TemplateDecl_)) TemplateDecl_ > \
		RetType_ Name_(const IEnumerable<ItemType>& enumerable STINGRAYKIT_COMMA_IF(STINGRAYKIT_NARGS(ParamsDecl_)) ParamsDecl_) \
		{ return Name_(*enumerable.GetEnumerator() STINGRAYKIT_COMMA_IF(STINGRAYKIT_NARGS(ParamsUsage_)) ParamsUsage_); } \
		template < typename EnumerableOrEnumerator, typename ItemType = typename EnumerableOrEnumerator::ItemType STINGRAYKIT_COMMA_IF(STINGRAYKIT_NARGS(TemplateDecl_)) TemplateDecl_, \
				typename EnableIf<IsEnumerator<EnumerableOrEnumerator>::Value || IsEnumerable<EnumerableOrEnumerator>::Value, int>::ValueT = 0 > \
		RetType_ Name_(const shared_ptr<EnumerableOrEnumerator>& enumerableOrEnumerator STINGRAYKIT_COMMA_IF(STINGRAYKIT_NARGS(ParamsDecl_)) ParamsDecl_) \
		{ \
			STINGRAYKIT_CHECK(enumerableOrEnumerator, NullArgumentException(IsEnumerator<EnumerableOrEnumerator>::Value ? "enumerator" : "enumerable")); \
			return Name_(*enumerableOrEnumerator STINGRAYKIT_COMMA_IF(STINGRAYKIT_NARGS(ParamsUsage_)) ParamsUsage_); \
		} \


		DETAIL_ENUMERABLE_HELPER_METHODS(MK_PARAM(typename AggregateFunc), ItemType, Aggregate, MK_PARAM(const AggregateFunc& aggregateFunc), MK_PARAM(aggregateFunc),
		{
			STINGRAYKIT_CHECK(enumerator.Valid(), InvalidOperationException());
			ItemType result = enumerator.Get();
			enumerator.Next();
			for (; enumerator.Valid(); enumerator.Next())
				result = FunctorInvoker::InvokeArgs(aggregateFunc, result, enumerator.Get());
			return result;
		})


		DETAIL_ENUMERABLE_HELPER_METHODS(MK_PARAM(typename ResultType, typename AggregateFunc), ResultType, Aggregate, MK_PARAM(const ResultType& initialValue, const AggregateFunc& aggregateFunc), MK_PARAM(initialValue, aggregateFunc),
		{
			ResultType result = initialValue;
			for (; enumerator.Valid(); enumerator.Next())
				result = FunctorInvoker::InvokeArgs(aggregateFunc, result, enumerator.Get());
			return result;
		})


		DETAIL_ENUMERABLE_HELPER_METHODS(MK_PARAM(typename PredicateFunc), bool, All, MK_PARAM(const PredicateFunc& predicate), MK_PARAM(predicate),
		{
			for (; enumerator.Valid(); enumerator.Next())
				if (!FunctorInvoker::InvokeArgs(predicate, enumerator.Get()))
					return false;
			return true;
		})


		DETAIL_ENUMERABLE_HELPER_METHODS(STINGRAYKIT_EMPTY(), bool, Any, STINGRAYKIT_EMPTY(), STINGRAYKIT_EMPTY(),
		{ return enumerator.Valid(); })

		DETAIL_ENUMERABLE_HELPER_METHODS(MK_PARAM(typename PredicateFunc), bool, Any, MK_PARAM(const PredicateFunc& predicate), MK_PARAM(predicate),
		{
			for (; enumerator.Valid(); enumerator.Next())
				if (FunctorInvoker::InvokeArgs(predicate, enumerator.Get()))
					return true;
			return false;
		})


		template < typename T >
		shared_ptr<IEnumerable<T>> Concat(const shared_ptr<IEnumerable<T>>& first, const shared_ptr<IEnumerable<T>>& second)
		{ return make_shared_ptr<Detail::JoiningEnumerable<T>>(first, second); }


		template < typename CastTo, typename SrcEnumerable >
		shared_ptr<IEnumerable<CastTo>> Cast(const shared_ptr<SrcEnumerable>& enumerable, typename EnableIf<IsEnumerable<SrcEnumerable>::Value, int>::ValueT dummy = 0)
		{ return Detail::EnumerableCaster<typename SrcEnumerable::ItemType>(enumerable); }


		DETAIL_ENUMERABLE_HELPER_METHODS(MK_PARAM(typename EqualPredicateFunc), bool, Contains, MK_PARAM(const ItemType& value, const EqualPredicateFunc& equalPredicate), MK_PARAM(value, equalPredicate),
		{
			for (; enumerator.Valid(); enumerator.Next())
				if (FunctorInvoker::InvokeArgs(equalPredicate, enumerator.Get(), value))
					return true;
			return false;
		})


		DETAIL_ENUMERABLE_HELPER_METHODS(STINGRAYKIT_EMPTY(), bool, Contains, MK_PARAM(const ItemType& value), MK_PARAM(value),
		{ return Contains(enumerator, value, std::equal_to<ItemType>()); })


		DETAIL_ENUMERABLE_HELPER_METHODS(STINGRAYKIT_EMPTY(), size_t, Count, STINGRAYKIT_EMPTY(), STINGRAYKIT_EMPTY(),
		{
			size_t result = 0;
			for (; enumerator.Valid(); enumerator.Next())
				++result;
			return result;
		})

		DETAIL_ENUMERABLE_HELPER_METHODS(MK_PARAM(typename PredicateFunc), size_t, Count, MK_PARAM(const PredicateFunc& predicate), MK_PARAM(predicate),
		{
			size_t result = 0;
			for (; enumerator.Valid(); enumerator.Next())
				if (FunctorInvoker::InvokeArgs(predicate, enumerator.Get()))
					++result;
			return result;
		})


		DETAIL_ENUMERABLE_HELPER_METHODS(MK_PARAM(typename Func), void, ForEach, MK_PARAM(const Func& func), MK_PARAM(func),
		{
			for (; enumerator.Valid(); enumerator.Next())
				FunctorInvoker::InvokeArgs(func, enumerator.Get());
		})


		DETAIL_ENUMERABLE_HELPER_METHODS(MK_PARAM(typename PredicateFunc, typename EnableIf<!IsConvertible<PredicateFunc, ItemType>::Value, int>::ValueT = 0), optional<size_t>, IndexOf, MK_PARAM(const PredicateFunc& predicate), MK_PARAM(predicate),
		{
			size_t result = 0;
			for (; enumerator.Valid(); enumerator.Next())
			{
				if (FunctorInvoker::InvokeArgs(predicate, enumerator.Get()))
					return result;
				++result;
			}
			return null;
		})

		DETAIL_ENUMERABLE_HELPER_METHODS(MK_PARAM(typename ValueType, typename EnableIf<IsConvertible<ValueType, ItemType>::Value, int>::ValueT = 0), optional<size_t>, IndexOf, MK_PARAM(const ValueType& val), MK_PARAM(val),
		{ return IndexOf(enumerator, Bind(std::equal_to<ItemType>(), val, _1)); })


		DETAIL_ENUMERABLE_HELPER_METHODS(STINGRAYKIT_EMPTY(), ItemType, ElementAt, MK_PARAM(size_t index), MK_PARAM(index),
		{
			size_t current = 0;
			for (; enumerator.Valid(); enumerator.Next(), ++current)
				if (index == current)
					return enumerator.Get();
			STINGRAYKIT_THROW(IndexOutOfRangeException(index, current));
		})

		DETAIL_ENUMERABLE_HELPER_METHODS(STINGRAYKIT_EMPTY(), ItemType, ElementAtOrDefault, MK_PARAM(size_t index), MK_PARAM(index),
		{
			size_t current = 0;
			for (; enumerator.Valid(); enumerator.Next(), ++current)
				if (index == current)
					return enumerator.Get();
			return ItemType();
		})


		template < typename T >
		shared_ptr<IEnumerable<T>> DefaultIfEmpty(const shared_ptr<IEnumerable<T>>& src, const T& value = T())
		{ return Any(src) ? src : MakeOneItemEnumerable(value); }


		template < typename T >
		shared_ptr<IEnumerable<T>> Empty()
		{ return MakeEmptyEnumerable(); }


		DETAIL_ENUMERABLE_HELPER_METHODS(STINGRAYKIT_EMPTY(), ItemType, First, STINGRAYKIT_EMPTY(), STINGRAYKIT_EMPTY(),
		{
			STINGRAYKIT_CHECK(enumerator.Valid(), InvalidOperationException());
			return enumerator.Get();
		})

		DETAIL_ENUMERABLE_HELPER_METHODS(MK_PARAM(typename PredicateFunc), ItemType, First, MK_PARAM(const PredicateFunc& predicate), MK_PARAM(predicate),
		{
			while (enumerator.Valid() && !FunctorInvoker::InvokeArgs(predicate, enumerator.Get()))
				enumerator.Next();
			STINGRAYKIT_CHECK(enumerator.Valid(), InvalidOperationException());
			return enumerator.Get();
		})


		DETAIL_ENUMERABLE_HELPER_METHODS(STINGRAYKIT_EMPTY(), ItemType, FirstOrDefault, STINGRAYKIT_EMPTY(), STINGRAYKIT_EMPTY(),
		{ return enumerator.Valid() ? enumerator.Get() : ItemType(); })

		DETAIL_ENUMERABLE_HELPER_METHODS(MK_PARAM(typename PredicateFunc), ItemType, FirstOrDefault, MK_PARAM(const PredicateFunc& predicate), MK_PARAM(predicate),
		{
			for (; enumerator.Valid(); enumerator.Next())
			{
				const ItemType result = enumerator.Get();
				if (FunctorInvoker::InvokeArgs(predicate, result))
					return result;
			}
			return ItemType();
		})


		DETAIL_ENUMERABLE_HELPER_METHODS(STINGRAYKIT_EMPTY(), ItemType, Last, STINGRAYKIT_EMPTY(), STINGRAYKIT_EMPTY(),
		{
			optional<ItemType> result;
			for (; enumerator.Valid(); enumerator.Next())
				result = enumerator.Get();
			STINGRAYKIT_CHECK(result, InvalidOperationException());
			return *result;
		})

		DETAIL_ENUMERABLE_HELPER_METHODS(MK_PARAM(typename PredicateFunc), ItemType, Last, MK_PARAM(const PredicateFunc& predicate), MK_PARAM(predicate),
		{
			optional<ItemType> result;
			for (; enumerator.Valid(); enumerator.Next())
			{
				const ItemType value = enumerator.Get();
				if (FunctorInvoker::InvokeArgs(predicate, value))
					result = value;
			}
			STINGRAYKIT_CHECK(result, InvalidOperationException());
			return *result;
		})


		DETAIL_ENUMERABLE_HELPER_METHODS(STINGRAYKIT_EMPTY(), ItemType, LastOrDefault, STINGRAYKIT_EMPTY(), STINGRAYKIT_EMPTY(),
		{
			optional<ItemType> result;
			for (; enumerator.Valid(); enumerator.Next())
				result = enumerator.Get();
			return result ? *result : ItemType();
		})

		DETAIL_ENUMERABLE_HELPER_METHODS(MK_PARAM(typename PredicateFunc), ItemType, LastOrDefault, MK_PARAM(const PredicateFunc& predicate), MK_PARAM(predicate),
		{
			optional<ItemType> result;
			for (; enumerator.Valid(); enumerator.Next())
			{
				const ItemType value = enumerator.Get();
				if (FunctorInvoker::InvokeArgs(predicate, value))
					result = value;
			}
			return result ? *result : ItemType();
		})


		namespace Detail
		{
			template < typename Dst_, typename SrcEnumerator_ >
			class EnumeratorOfType : public virtual IEnumerator<Dst_>
			{
				using Storage = RefStorage<Dst_>;

			private:
				SrcEnumerator_              _srcEnumerator;
				typename Storage::ValueType _dst;

			public:
				EnumeratorOfType(const SrcEnumerator_& srcEnumerator) : _srcEnumerator(srcEnumerator)
				{ FindNext(); }

				bool Valid() const override
				{ return _srcEnumerator->Valid(); }

				Dst_ Get() const override
				{
					STINGRAYKIT_CHECK(_srcEnumerator->Valid(), "Enumerator is not valid!");
					return Storage::Unwrap(_dst);
				}

				void Next() override
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
		shared_ptr<IEnumerator<TResult>> OfType(const shared_ptr<SrcEnumerator>& enumerator, typename EnableIf<IsEnumerator<SrcEnumerator>::Value, int>::ValueT dummy = 0)
		{ return make_shared_ptr<Detail::EnumeratorOfType<TResult, shared_ptr<SrcEnumerator>>>(enumerator); }


		template < typename TResult, typename SrcEnumerable >
		shared_ptr<IEnumerable<TResult>> OfType(const shared_ptr<SrcEnumerable>& enumerable, typename EnableIf<IsEnumerable<SrcEnumerable>::Value, int>::ValueT dummy = 0)
		{ return MakeSimpleEnumerable(Bind(MakeShared<Detail::EnumeratorOfType<TResult, shared_ptr<IEnumerator<typename SrcEnumerable::ItemType>>>>(), Bind(&SrcEnumerable::GetEnumerator, enumerable))); }


		template < typename CollectionType >
		shared_ptr<IEnumerator<typename CollectionType::ItemType>> Reverse(const shared_ptr<CollectionType>& enumerator, typename EnableIf<IsEnumerator<CollectionType>::Value, int>::ValueT dummy = 0)
		{
			const shared_ptr<std::vector<typename CollectionType::ItemType>> result = make_shared_ptr<std::vector<typename CollectionType::ItemType>>();
			for (; enumerator.Valid(); enumerator.Next())
				result->push_back(enumerator.Get());
			return EnumeratorFromStlIterators(result->rbegin(), result->rend(), result);
		}

		template < typename CollectionType >
		shared_ptr<IEnumerable<typename CollectionType::ItemType>> Reverse(const shared_ptr<CollectionType>& enumerable, typename EnableIf<IsEnumerable<CollectionType>::Value && !IsReversableEnumerable<CollectionType>::Value, int>::ValueT dummy = 0)
		{
			const shared_ptr<std::vector<typename CollectionType::ItemType>> result = make_shared_ptr<std::vector<typename CollectionType::ItemType>>();
			for (shared_ptr<IEnumerator<typename CollectionType::ItemType>> enumerator = enumerable->GetEnumerator(); enumerator->Valid(); enumerator->Next())
				result->push_back(enumerator->Get());
			return EnumerableFromStlIterators(result->rbegin(), result->rend(), result);
		}

		template < typename CollectionType >
		shared_ptr<IEnumerable<typename CollectionType::ItemType>> Reverse(const shared_ptr<CollectionType>& enumerable, typename EnableIf<IsEnumerable<CollectionType>::Value && IsReversableEnumerable<CollectionType>::Value, int>::ValueT dummy = 0)
		{ return enumerable->Reverse(); }


		DETAIL_ENUMERABLE_HELPER_METHODS(STINGRAYKIT_EMPTY(), ItemType, Single, STINGRAYKIT_EMPTY(), STINGRAYKIT_EMPTY(),
		{
			STINGRAYKIT_CHECK(enumerator.Valid(), InvalidOperationException());
			const ItemType result = enumerator.Get();
			enumerator.Next();
			STINGRAYKIT_CHECK(!enumerator.Valid(), InvalidOperationException());
			return result;
		})


		namespace Detail
		{
			template < typename SrcType, typename FunctorType >
			class EnumeratorTransformer : public virtual IEnumerator<typename function_info<FunctorType>::RetType>
			{
				using base = IEnumerator<typename function_info<FunctorType>::RetType>;
				using SrcEnumeratorPtr = shared_ptr<IEnumerator<SrcType>>;

			private:
				SrcEnumeratorPtr	_src;
				FunctorType			_functor;

			public:
				EnumeratorTransformer(const SrcEnumeratorPtr& src, const FunctorType& functor)
					:	_src(STINGRAYKIT_REQUIRE_NOT_NULL(src)),
						_functor(functor)
				{ }

				bool Valid() const override						{ return _src->Valid(); }
				typename base::ItemType Get() const override	{ return FunctorInvoker::InvokeArgs(_functor, _src->Get()); }
				void Next() override							{ _src->Next(); }
			};

			template < typename SrcType, typename FunctorType >
			class EnumerableTransformer : public virtual IEnumerable<typename function_info<FunctorType>::RetType>
			{
				using base = IEnumerable<typename function_info<FunctorType>::RetType>;
				using SrcEnumerablePtr = shared_ptr<IEnumerable<SrcType>>;

			private:
				SrcEnumerablePtr	_src;
				FunctorType			_functor;

			public:
				EnumerableTransformer(const SrcEnumerablePtr& src, const FunctorType& functor)
					:	_src(STINGRAYKIT_REQUIRE_NOT_NULL(src)),
						_functor(functor)
				{ }

				shared_ptr<IEnumerator<typename base::ItemType>> GetEnumerator() const override
				{ return make_shared_ptr<EnumeratorTransformer<SrcType, FunctorType>>(_src->GetEnumerator(), _functor); }
			};
		}


		template < typename SrcEnumerator, typename FunctorType >
		shared_ptr<IEnumerator<typename function_info<FunctorType>::RetType>> Transform(const shared_ptr<SrcEnumerator>& enumerator, const FunctorType& functor, typename EnableIf<IsEnumerator<SrcEnumerator>::Value, int>::ValueT dummy = 0)
		{ return make_shared_ptr<Detail::EnumeratorTransformer<typename SrcEnumerator::ItemType, FunctorType>>(enumerator, functor); }


		template < typename SrcEnumerable, typename FunctorType >
		shared_ptr<IEnumerable<typename function_info<FunctorType>::RetType>> Transform(const shared_ptr<SrcEnumerable>& enumerable, const FunctorType& functor, typename EnableIf<IsEnumerable<SrcEnumerable>::Value, int>::ValueT dummy = 0)
		{ return make_shared_ptr<Detail::EnumerableTransformer<typename SrcEnumerable::ItemType, FunctorType>>(enumerable, functor); }


		template < typename CollectionType, typename PredicateFunc >
		shared_ptr<IEnumerator<typename CollectionType::ItemType>> Where(const shared_ptr<CollectionType>& enumerator, const PredicateFunc& predicate, typename EnableIf<IsEnumerator<CollectionType>::Value, int>::ValueT dummy = 0)
		{
			using ItemType = typename CollectionType::ItemType;
			return WrapEnumerator(enumerator, &stingray::implicit_cast<ItemType>, predicate);
		}

		template < typename CollectionType, typename PredicateFunc >
		shared_ptr<IEnumerable<typename CollectionType::ItemType>> Where(const shared_ptr<CollectionType>& enumerable, const PredicateFunc& predicate, typename EnableIf<IsEnumerable<CollectionType>::Value, int>::ValueT dummy = 0)
		{
			using ItemType = typename CollectionType::ItemType;
			return WrapEnumerable(enumerable, &stingray::implicit_cast<ItemType>, predicate);
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
			class EnumeratorTaker : public virtual IEnumerator<ItemType>
			{
				using SrcEnumeratorPtr = shared_ptr<IEnumerator<ItemType>>;

			private:
				SrcEnumeratorPtr	_src;
				size_t				_index;
				size_t				_count;

			public:
				EnumeratorTaker(const SrcEnumeratorPtr& src, size_t count)
					:	_src(STINGRAYKIT_REQUIRE_NOT_NULL(src)),
						_index(0),
						_count(count)
				{ }

				bool Valid() const override
				{ return _src->Valid() && _index < _count; }

				ItemType Get() const override
				{
					STINGRAYKIT_CHECK(Valid(), "Enumerator is not valid!");
					return _src->Get();
				}

				void Next() override
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


		namespace Detail
		{
			template < typename SrcType >
			class EnumeratorFlattener : public virtual IEnumerator<typename SrcType::ValueType::ItemType>
			{
				using base = IEnumerator<typename SrcType::ValueType::ItemType>;

				using SrcEnumeratorPtr = shared_ptr<IEnumerator<SrcType>>;
				using DstEnumeratorPtr = shared_ptr<IEnumerator<typename base::ItemType>>;

			private:
				SrcEnumeratorPtr	_src;
				DstEnumeratorPtr	_current;

			public:
				EnumeratorFlattener(const SrcEnumeratorPtr& src)
					:	_src(STINGRAYKIT_REQUIRE_NOT_NULL(src))
				{ FindNext(); }

				bool Valid() const override
				{ return _current && _current->Valid(); }

				typename base::ItemType Get() const override
				{
					STINGRAYKIT_CHECK(_current, "Enumerator is not valid!");
					return _current->Get();
				}

				void Next() override
				{
					STINGRAYKIT_CHECK(_current, "Enumerator is not valid!");

					_current->Next();
					if (_current->Valid())
						return;

					_current.reset();
					_src->Next();
					FindNext();
				}

			private:
				void FindNext()
				{
					while (_src->Valid())
					{
						_current = ToEnumerator(_src->Get());
						if (_current->Valid())
							break;

						_current.reset();
						_src->Next();
					}
				}
			};
		}


		template < typename SrcEnumerator >
		shared_ptr<IEnumerator<typename SrcEnumerator::ItemType::ValueType::ItemType>> Flatten(const shared_ptr<SrcEnumerator>& enumeratorOfEnumerators, typename EnableIf<IsEnumerator<SrcEnumerator>::Value, int>::ValueT dummy = 0)
		{ return make_shared_ptr<Detail::EnumeratorFlattener<typename SrcEnumerator::ItemType> >(enumeratorOfEnumerators); }


		template < typename SrcEnumerable >
		shared_ptr<IEnumerable<typename SrcEnumerable::ItemType::ValueType::ItemType>> Flatten(const shared_ptr<SrcEnumerable>& enumerableOfEnumerables, typename EnableIf<IsEnumerable<SrcEnumerable>::Value, int>::ValueT dummy = 0)
		{ return MakeSimpleEnumerable(Bind(MakeShared<Detail::EnumeratorFlattener<typename SrcEnumerable::ItemType>>(), Bind(&SrcEnumerable::GetEnumerator, enumerableOfEnumerables))); }


		template < typename T, typename PredicateFunc >
		bool SequenceEqual(const shared_ptr<IEnumerator<T>>& first, const shared_ptr<IEnumerator<T>>& second, const PredicateFunc& equalPredicate)
		{
			for (; first->Valid() && second->Valid(); first->Next(), second->Next())
				if (!FunctorInvoker::InvokeArgs(equalPredicate, first->Get(), second->Get()))
					return false;
			return !first->Valid() && !second->Valid();
		}


		template < typename T >
		bool SequenceEqual(const shared_ptr<IEnumerator<T>>& first, const shared_ptr<IEnumerator<T>>& second)
		{ return SequenceEqual(first, second, std::equal_to<T>()); }


		template < typename First, typename Second, typename PredicateFunc >
		bool SequenceEqual(const shared_ptr<First>& first, const shared_ptr<Second>& second, const PredicateFunc& equalPredicate, typename EnableIf<!IsEnumerator<First>::Value || !IsEnumerator<Second>::Value, int>::ValueT dummy = 0)
		{ return SequenceEqual(ToEnumerator(first), ToEnumerator(second), equalPredicate); }


		template < typename First, typename Second >
		bool SequenceEqual(const shared_ptr<First>& first, const shared_ptr<Second>& second, typename EnableIf<!IsEnumerator<First>::Value || !IsEnumerator<Second>::Value, int>::ValueT dummy = 0)
		{ return SequenceEqual(ToEnumerator(first), ToEnumerator(second)); }


#undef DETAIL_ENUMERABLE_HELPER_METHODS

#endif

	}


	template < typename Enumerable_ >
	struct FirstTransformerImpl<shared_ptr<Enumerable_>, typename EnableIf<IsEnumerable<Enumerable_>::Value, void>::ValueT>
	{
		using ValueT = typename Enumerable_::ItemType;

		static ValueT Do(const shared_ptr<Enumerable_>& enumerable, const FirstTransformer& action)
		{ return Enumerable::First(enumerable); }
	};


	template < typename Enumerable_ >
	struct FirstOrDefaultTransformerImpl<shared_ptr<Enumerable_>, typename EnableIf<IsEnumerable<Enumerable_>::Value, void>::ValueT>
	{
		using ValueT = typename Enumerable_::ItemType;

		static ValueT Do(const shared_ptr<Enumerable_>& enumerable, const FirstOrDefaultTransformer& action)
		{ return Enumerable::FirstOrDefault(enumerable); }
	};


	template < typename Enumerable_, typename Predicate_ >
	struct FilterTransformerImpl<shared_ptr<Enumerable_>, Predicate_, typename EnableIf<IsEnumerable<Enumerable_>::Value, void>::ValueT>
	{
		using ItemType = typename Enumerable_::ItemType;
		using ValueT = shared_ptr<IEnumerable<ItemType>>;

		static ValueT Do(const shared_ptr<Enumerable_>& enumerable, const FilterTransformer<Predicate_>& action)
		{ return Enumerable::Where(enumerable, action.GetPredicate()); }
	};


	template < typename Enumerable_ >
	struct ReverseTransformerImpl<shared_ptr<Enumerable_>, typename EnableIf<IsEnumerable<Enumerable_>::Value, void>::ValueT>
	{
		using ItemType = typename Enumerable_::ItemType;
		using ValueT = shared_ptr<IEnumerable<ItemType>>;

		static ValueT Do(const shared_ptr<Enumerable_>& enumerable, const ReverseTransformer& action)
		{ return Enumerable::Reverse(enumerable); }
	};


	template < typename Enumerable_, typename Functor_ >
	struct TransformTransformerImpl<shared_ptr<Enumerable_>, Functor_, typename EnableIf<IsEnumerable<Enumerable_>::Value, void>::ValueT>
	{
		using ItemType = typename function_info<Functor_>::RetType;
		using ValueT = shared_ptr<IEnumerable<ItemType>>;

		static ValueT Do(const shared_ptr<Enumerable_>& enumerable, const TransformTransformer<Functor_>& action)
		{ return Enumerable::Transform(enumerable, action.GetFunctor()); }
	};


	template < typename Enumerable_, typename Dst_ >
	struct CastTransformerImpl<shared_ptr<Enumerable_>, Dst_, typename EnableIf<IsEnumerable<Enumerable_>::Value, void>::ValueT>
	{
		using ValueT = shared_ptr<IEnumerable<Dst_>>;

		static ValueT Do(const shared_ptr<Enumerable_>& enumerable, const CastTransformer<Dst_>& action)
		{ return Enumerable::Cast<Dst_>(enumerable); }
	};


	template < typename Enumerable_, typename Dst_ >
	struct OfTypeTransformerImpl<shared_ptr<Enumerable_>, Dst_, typename EnableIf<IsEnumerable<Enumerable_>::Value, void>::ValueT>
	{
		using ValueT = shared_ptr<IEnumerable<Dst_>>;

		static ValueT Do(const shared_ptr<Enumerable_>& enumerable, const OfTypeTransformer<Dst_>& action)
		{ return Enumerable::OfType<Dst_>(enumerable); }
	};


	template < typename Enumerable_ >
	struct AnyTransformerImpl<shared_ptr<Enumerable_>, typename EnableIf<IsEnumerable<Enumerable_>::Value, void>::ValueT>
	{
		using ValueT = bool;

		static ValueT Do(const shared_ptr<Enumerable_>& enumerable, const AnyTransformer& action)
		{ return Enumerable::Any(enumerable); }
	};


	template < typename Enumerable_ >
	struct CountTransformerImpl<shared_ptr<Enumerable_>, typename EnableIf<IsEnumerable<Enumerable_>::Value, void>::ValueT>
	{
		using ValueT = size_t;

		static ValueT Do(const shared_ptr<Enumerable_>& enumerable, const CountTransformer& action)
		{ return Enumerable::Count(enumerable); }
	};


	template < typename Enumerable_ >
	struct DropTransformerImpl<shared_ptr<Enumerable_>, typename EnableIf<IsEnumerable<Enumerable_>::Value, void>::ValueT>
	{
		using ItemType = typename Enumerable_::ItemType;
		using ValueT = shared_ptr<IEnumerable<ItemType>>;

		static ValueT Do(const shared_ptr<Enumerable_>& enumerable, const DropTransformer& action)
		{ return Enumerable::Skip(enumerable, action.GetCount()); }
	};


	template < typename Enumerable_ >
	struct TakeTransformerImpl<shared_ptr<Enumerable_>, typename EnableIf<IsEnumerable<Enumerable_>::Value, void>::ValueT>
	{
		using ItemType = typename Enumerable_::ItemType;
		using ValueT = shared_ptr<IEnumerable<ItemType>>;

		static ValueT Do(const shared_ptr<Enumerable_>& enumerable, const TakeTransformer& action)
		{ return Enumerable::Take(enumerable, action.GetCount()); }
	};

	/** @} */

}

#endif
