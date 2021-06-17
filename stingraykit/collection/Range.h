#ifndef STINGRAYKIT_COLLECTION_RANGE_H
#define STINGRAYKIT_COLLECTION_RANGE_H

// Copyright (c) 2011 - 2019, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/RefStorage.h>
#include <stingraykit/collection/RangeBase.h>
#include <stingraykit/collection/ToRange.h>
#include <stingraykit/collection/Transformers.h>
#include <stingraykit/function/FunctorInvoker.h>
#include <stingraykit/dynamic_caster.h>
#include <stingraykit/math.h>
#include <stingraykit/optional.h>

namespace stingray
{
	namespace Range
	{
		//template <typename ValueType_>
		//struct ForwardRange
		//{
		//    typedef ValueType_ ValueType;
		//
		//    bool Valid() const;
		//    ValueType Get();
		//
		//    bool Equals(const Self& other) const;
		//
		//    Self& First();
		//    Self& Next();
		//
		//    ValueType operator * ()                       { return Get(); }
		//    ArrowProxy<ValueType_> operator -> ()         { return Get(); }
		//
		//    Self& operator ++ ()                          { Next(); return *this; }
		//    Self operator ++ (int)                        { Self result(*this); Next(); return result; }
		//
		//    bool operator == (const Self& other) const    { return Equals(other); }
		//    bool operator != (const Self& other) const    { return !Equals(other); }
		//};


		//template <typename ValueType_>
		//struct BidirectionalRange : ForwardRange<ValueType_>
		//{
		//    Self& Last();
		//    Self& Prev();
		//
		//    Self& operator -- ()   { Prev(); return *this; }
		//    Self operator -- (int) { Self result(*this); Prev(); return result; }
		//};


		//template <typename ValueType_>
		//struct RandomAccessRange : BidirectionalRange<ValueType_>
		//{
		//    std::ptrdiff_t GetPosition() const;
		//    size_t GetSize() const;
		//    Self& Move(std::ptrdiff_t distance);
		//
		//    ValueType operator [] (std::ptrdiff_t index) const  { static_assert(false, "No brackets operator"); }
		//    Self& operator += (std::ptrdiff_t distance)         { Move(distance); return *this; }
		//    Self operator + (std::ptrdiff_t distance) const     { Self result(*this); return result += distance; }
		//    Self& operator -= (std::ptrdiff_t distance)         { Move(distance); return *this; }
		//    Self operator - (std::ptrdiff_t distance) const     { Self result(*this); return result -= distance; }
		//    std::ptrdiff_t operator - (const Self& other) const { return GetPosition() - other.GetPosition(); }
		//};


		template < typename Iterator_ >
		class OutputIteratorRange
		{
		public:
			typedef typename std::iterator_traits<Iterator_>::reference ValueType;

		private:
			Iterator_ _it;

		public:
			OutputIteratorRange(const Iterator_& it) : _it(it) { }

			bool Valid() const	{ return true; }
			ValueType Get()		{ return *_it; }

			void First()		{ static_assert(sizeof(Iterator_) < 0, "No first in output range"); }
			void Next()			{ ++_it; }
		};


		template < typename It_ >
		class IteratorRange :
			public RangeBase<IteratorRange<It_>, typename std::iterator_traits<It_>::reference, typename std::iterator_traits<It_>::iterator_category>
		{
			typedef IteratorRange<It_> Self;
			typedef RangeBase<IteratorRange<It_>, typename std::iterator_traits<It_>::reference, typename std::iterator_traits<It_>::iterator_category> base;

		private:
			const It_		_begin;
			It_				_it;
			const It_		_end;

		public:
			IteratorRange(const It_& begin, const It_& it, const It_& end) : _begin(begin), _it(it), _end(end) { }

			bool Valid() const
			{ return _it != _end; }

			typename base::ValueType Get()
			{ STINGRAYKIT_CHECK(Valid(), "Get() behind last element"); return *_it; }

			bool Equals(const IteratorRange& other) const
			{ return _begin == other._begin && _it == other._it && _end == other._end; }

			Self& First()
			{ _it = _begin; return *this; }

			Self& Next()
			{ STINGRAYKIT_CHECK(Valid(), "Next() behind last element"); ++_it; return *this; }

			Self& Last()
			{ _it = _end; if (_it != _begin) --_it; return *this; }

			Self& Prev()
			{ STINGRAYKIT_CHECK(_it != _begin, "Prev() at first element"); --_it; return *this; }

			size_t GetPosition() const
			{ return std::distance(_begin, _it); }

			size_t GetSize() const
			{ return std::distance(_begin, _end); }

			Self& Move(int distance)
			{
				STINGRAYKIT_CHECK(GetPosition() + distance <= GetSize(), IndexOutOfRangeException(GetPosition() + distance, GetSize()));
				std::advance(_it, distance);
				return *this;
			}
		};


		/// @brief Since filtering items removes random-access property from range, we need to appropriately change category
		template < typename Category_ >
		struct RangeFilterCategoryHelper
		{ typedef Category_ ValueT; };


		template < >
		struct RangeFilterCategoryHelper<std::random_access_iterator_tag>
		{ typedef std::bidirectional_iterator_tag ValueT; };


		template < typename Range_, typename Predicate_ >
		class RangeFilter : public RangeBase<RangeFilter<Range_, Predicate_>, typename Range_::ValueType, typename RangeFilterCategoryHelper<typename Range_::Category>::ValueT>
		{
			typedef RangeBase<RangeFilter<Range_, Predicate_>, typename Range_::ValueType, typename RangeFilterCategoryHelper<typename Range_::Category>::ValueT> base;
			typedef RangeFilter<Range_, Predicate_> Self;

		public:
			static const bool ReturnsTemporary = Range_::ReturnsTemporary;

		private:
			Range_		_impl;
			Predicate_	_predicate;

		public:
			RangeFilter(const Range_& impl, const Predicate_& predicate)
				: _impl(impl), _predicate(predicate)
			{ FindNext(); }

			bool Valid() const								{ return _impl.Valid(); }
			typename base::ValueType Get()					{ return _impl.Get(); }
			bool Equals(const RangeFilter& other) const		{ return _impl == other._impl; }
			Self& First()									{ _impl.First(); FindNext(); return *this; }
			Self& Last()									{ _impl.Last(); FindPrev(); return *this; }
			Self& Next()									{ _impl.Next(); FindNext(); return *this; }
			Self& Prev()									{ _impl.Prev(); FindPrev(); return *this; }

		private:
			void FindNext()
			{
				while (_impl.Valid() && !FunctorInvoker::InvokeArgs(_predicate, _impl.Get()))
					_impl.Next();
			}

			void FindPrev()
			{
				while (_impl.Valid() && !FunctorInvoker::InvokeArgs(_predicate, _impl.Get()))
					_impl.Prev();
			}
		};


		template < typename Dst_, typename Range_ >
		class RangeCaster : public RangeBase<RangeCaster<Dst_, Range_>, Dst_, typename Range_::Category>
		{
			typedef RangeCaster<Dst_, Range_> Self;
			typedef RangeBase<RangeCaster<Dst_, Range_>, Dst_, typename Range_::Category> base;

		public:
			static const bool ReturnsTemporary = Range_::ReturnsTemporary;

		private:
			Range_			_impl;
			optional<Dst_>	_cache;

		public:
			RangeCaster(const Range_& impl) : _impl(impl) { }

			bool Valid() const								{ return _impl.Valid(); }
			typename base::ValueType Get()					{ DoCast(); return *_cache; }
			bool Equals(const RangeCaster& other) const		{ return _impl == other._impl; }
			Self& First()									{ _impl.First(); _cache.reset(); return *this; }
			Self& Next()									{ _impl.Next(); _cache.reset(); return *this; }
			Self& Last()									{ _impl.Last(); _cache.reset(); return *this; }
			Self& Prev()									{ _impl.Prev(); _cache.reset(); return *this; }

			size_t GetPosition() const						{ return _impl.GetPosition(); }
			size_t GetSize() const							{ return _impl.GetSize(); }
			Self& Move(int distance)						{ _impl.Move(distance); _cache.reset(); return *this; }

		private:
			void DoCast()
			{
				if (!_cache)
				{
					Dst_ temp = STINGRAYKIT_CHECKED_DYNAMIC_CASTER(STINGRAYKIT_REQUIRE_NOT_NULL(_impl.Get()));
					_cache = temp;
				}
			}
		};


		template < typename Dst_, typename Range_ >
		class RangeOfType : public RangeBase<RangeOfType<Dst_, Range_>, Dst_, typename RangeFilterCategoryHelper<typename Range_::Category>::ValueT>
		{
			typedef RangeOfType<Dst_, Range_> Self;
			typedef RangeBase<RangeOfType<Dst_, Range_>, Dst_, typename RangeFilterCategoryHelper<typename Range_::Category>::ValueT> base;
			typedef RefStorage<Dst_> Storage;

		public:
			static const bool ReturnsTemporary = Range_::ReturnsTemporary;

		private:
			Range_							_impl;
			typename Storage::ValueType		_dst;

		public:
			RangeOfType(const Range_& impl) : _impl(impl) { FindNext(); }

			bool Valid() const								{ return _impl.Valid(); }
			typename base::ValueType Get()					{ return Storage::Unwrap(_dst); }
			bool Equals(const RangeOfType& other) const		{ return _impl == other._impl; }
			Self& First()									{ _impl.First(); FindNext(); return *this; }
			Self& Next()									{ _impl.Next(); FindNext(); return *this; }
			Self& Last()									{ _impl.Last(); FindPrev(); return *this; }
			Self& Prev()									{ _impl.Prev(); FindPrev(); return *this; }

		private:
			void FindNext()
			{
				for ( ; _impl.Valid(); _impl.Next())
				{
					_dst = DynamicCast<typename Storage::ValueType>(Storage::Wrap(_impl.Get()));
					if (_dst)
						return;
				}
			}

			void FindPrev()
			{
				for ( ; _impl.Valid(); _impl.Prev())
				{
					_dst = DynamicCast<typename Storage::ValueType>(Storage::Wrap(_impl.Get()));
					if (_dst)
						return;
				}
			}
		};


		template < typename Range_ >
		class RangeReverser : public RangeBase<RangeReverser<Range_>, typename Range_::ValueType, typename Range_::Category>
		{
			typedef RangeReverser<Range_> Self;
			typedef RangeBase<RangeReverser<Range_>, typename Range_::ValueType, typename Range_::Category> base;

		public:
			static const bool ReturnsTemporary = Range_::ReturnsTemporary;

		private:
			Range_		_impl;
			bool		_implBeforeBegin;

		public:
			RangeReverser(const Range_& impl)
				: _impl(impl)
			{ First(); }

			bool Valid() const
			{ return !_implBeforeBegin && _impl.Valid(); }

			typename base::ValueType Get()
			{ STINGRAYKIT_CHECK(!_implBeforeBegin, "range at the end!"); return _impl.Get(); }

			bool Equals(const RangeReverser& other) const
			{ return _impl == other._impl && _implBeforeBegin == other._implBeforeBegin; }

			Self& First()
			{
				_impl.Last();
				_implBeforeBegin = false;
				return *this;
			}

			Self& Next()
			{
				if (_impl != Range_(_impl).First())
					_impl.Prev();
				else
					_implBeforeBegin = true;
				return *this;
			}

			Self& Last()
			{
				_impl.First();
				_implBeforeBegin = false;
				return *this;
			}

			Self& Prev()
			{
				if (_implBeforeBegin)
					_implBeforeBegin = false;
				else
					_impl.Next();
				return *this;
			}

			size_t GetPosition() const
			{ return static_cast<int>(_impl.GetSize()) - static_cast<int>(_impl.GetPosition()) - (_implBeforeBegin ? 0 : 1); }

			size_t GetSize() const
			{ return _impl.GetSize(); }

			Self& Move(int distance)
			{ _impl.Move(-distance); return *this; }
		};


		template < typename Range_, typename Functor_ >
		class RangeTransformer : public RangeBase<RangeTransformer<Range_, Functor_>, typename AddConstLvalueReference<typename function_info<Functor_>::RetType>::ValueT, typename Range_::Category>
		{
			typedef RangeTransformer<Range_, Functor_> Self;
			typedef RangeBase<RangeTransformer<Range_, Functor_>, typename AddConstLvalueReference<typename function_info<Functor_>::RetType>::ValueT, typename Range_::Category> base;

		public:
			static const bool ReturnsTemporary = true;

		private:
			Range_												_impl;
			Functor_											_functor;
			optional<typename function_info<Functor_>::RetType>	_cache;

		public:
			RangeTransformer(const Range_& impl, const Functor_& functor)
				: _impl(impl), _functor(functor)
			{ }

			bool Valid() const									{ return _impl.Valid(); }
			typename base::ValueType Get()						{ DoTransform(); return *_cache; }
			bool Equals(const RangeTransformer& other) const	{ return _impl == other._impl; }
			Self& First()										{ _impl.First(); _cache.reset(); return *this; }
			Self& Next()										{ _impl.Next(); _cache.reset(); return *this; }
			Self& Last()										{ _impl.Last(); _cache.reset(); return *this; }
			Self& Prev()										{ _impl.Prev(); _cache.reset(); return *this; }

			size_t GetPosition() const							{ return _impl.GetPosition(); }
			size_t GetSize() const								{ return _impl.GetSize(); }
			Self& Move(int distance)							{ _impl.Move(distance); _cache.reset(); return *this; }

		private:
			void DoTransform()
			{
				if (!_cache)
					_cache.emplace(FunctorInvoker::InvokeArgs(_functor, _impl.Get()));
			}
		};


		template < typename Range_ >
		class RangeTaker : public RangeBase<RangeTaker<Range_>, typename Range_::ValueType, typename Range_::Category>
		{
			typedef RangeBase<RangeTaker<Range_>, typename Range_::ValueType, typename Range_::Category> base;
			typedef RangeTaker<Range_> Self;

		public:
			static const bool ReturnsTemporary = Range_::ReturnsTemporary;

		private:
			const Range_		_initial;
			optional<Range_	>	_impl;
			size_t				_index;
			const size_t		_count;

		public:
			RangeTaker(const Range_& impl, size_t count)
				: _initial(impl), _impl(impl), _index(0), _count(count)
			{ }

			bool Valid() const
			{ return _impl->Valid() && _index < _count; }

			typename base::ValueType Get()
			{
				STINGRAYKIT_CHECK(Valid(), "Get() behind last element");
				return _impl->Get();
			}

			bool Equals(const RangeTaker& other) const
			{ return _initial == other._initial && _impl == other._impl && _index == other._index && _count == other._count; }

			Self& First()
			{
				_impl.emplace(_initial);
				_index = 0;
				return *this;
			}

			Self& Last()
			{
				First();
				Self prev(*this);

				while (Valid())
				{
					Next();
					if (Valid())
						prev.Next();
				}

				_impl.emplace(*prev._impl);
				_index = prev._index;
				return *this;
			}

			Self& Next()
			{
				STINGRAYKIT_CHECK(Valid(), "Next() behind last element");
				_impl->Next();
				++_index;
				return *this;
			}

			Self& Prev()
			{
				STINGRAYKIT_CHECK(_index > 0, "Prev() at first element");
				_impl->Prev();
				--_index;
				return *this;
			}

			size_t GetPosition() const
			{ return _index; }

			size_t GetSize() const
			{ return std::min(_initial.GetSize() - _initial.GetPosition(), _count); }

			Self& Move(int distance)
			{
				STINGRAYKIT_CHECK(GetPosition() + distance <= GetSize(), IndexOutOfRangeException(GetPosition() + distance, GetSize()));
				_impl->Move(distance);
				_index += distance;
				return *this;
			}
		};


		/// Doesn't support neither random-access nor equality check for now
		template < typename Range_ >
		class RangeCycler : public RangeBase<RangeCycler<Range_>, typename Range_::ValueType, typename RangeFilterCategoryHelper<typename Range_::Category>::ValueT>
		{
			typedef RangeCycler<Range_> Self;
			typedef RangeBase<RangeCycler<Range_>, typename Range_::ValueType, typename RangeFilterCategoryHelper<typename Range_::Category>::ValueT> base;

		private:
			Range_ _impl;

		public:
			RangeCycler(const Range_& impl)
				: _impl(impl)
			{ STINGRAYKIT_CHECK(_impl.Valid(), "Can't cycle empty range!"); }

			bool Valid() const
			{ return true; }

			typename base::ValueType Get()
			{ return _impl.Get(); }

			bool Equals(const RangeCycler& other) const
			{ return _impl == other._impl; }

			Self& First()
			{
				_impl.First();
				CheckValid();
				return *this;
			}

			Self& Next()
			{
				_impl.Next();
				if (_impl.Valid())
					return *this;

				_impl.First();
				CheckValid();
				return *this;
			}

			Self& Last()
			{
				_impl.Last();
				CheckValid();
				return *this;
			}

			Self& Prev()
			{
				if (_impl == Range_(_impl).First())
					_impl.Last();
				else
					_impl.Prev();
				CheckValid();
				return *this;
			}

		protected:
			void CheckValid() const
			{ STINGRAYKIT_CHECK(_impl.Valid(), LogicException("Something is terribly wrong with internal range!")); }
		};


		template < typename It_ >
		class RangeSplitter : public Range::RangeBase<RangeSplitter<It_>, Range::IteratorRange<It_>, std::forward_iterator_tag>
		{
			typedef RangeSplitter<It_> Self;
			typedef Range::RangeBase<RangeSplitter<It_>, Range::IteratorRange<It_>, std::forward_iterator_tag> base;

			typedef typename std::iterator_traits<It_>::difference_type DiffType;

		private:
			const It_							_begin;
			const It_							_end;
			const DiffType						_maxFragmentSize;
			It_									_it;
			optional<typename base::ValueType>	_value;

		public:
			RangeSplitter(const It_& begin, const It_& end, DiffType maxFragmentSize)
				: _begin(begin), _end(end), _maxFragmentSize(maxFragmentSize), _it(_begin)
			{ }

			bool Valid() const
			{ return _it != _end; }

			typename base::ValueType Get()
			{
				STINGRAYKIT_CHECK(Valid(), "Get() behind last element");
				if (!_value)
					_value.emplace(_it, _it, std::next(_it, GetSize()));
				return *_value;
			}

			bool Equals(const RangeSplitter& other) const
			{ return _begin == other._begin && _end == other._end && _it == other._it && _maxFragmentSize == other._maxFragmentSize; }

			Self& First()
			{
				_it = _begin;
				_value.reset();
				return *this;
			}

			Self& Next()
			{
				STINGRAYKIT_CHECK(Valid(), "Next() behind last element");
				std::advance(_it, GetSize());
				_value.reset();
				return *this;
			}

			Self& Prev()
			{
				STINGRAYKIT_CHECK(_it != _begin, "Prev() at first element");
				_it = std::next(_begin, AlignDown(std::distance(_begin, _it) - 1, _maxFragmentSize));
				_value.reset();
				return *this;
			}

			Self& Last()
			{
				_it = std::next(_begin, AlignDown(std::distance(_begin, _end), _maxFragmentSize));
				_value.reset();
				return *this;
			}

		private:
			DiffType GetSize() const
			{ return std::min<DiffType>(_maxFragmentSize, std::distance(_it, _end)); }
		};


		template < typename FuncType_, typename RangeTypes_ >
		class RangeZipper : public Range::RangeBase<RangeZipper<FuncType_, RangeTypes_>, typename function_info<FuncType_>::RetType, std::forward_iterator_tag>
		{
			typedef RangeZipper<FuncType_, RangeTypes_> Self;
			typedef Range::RangeBase<RangeZipper<FuncType_, RangeTypes_>, typename function_info<FuncType_>::RetType, std::forward_iterator_tag> base;

			static const size_t RangeCount = GetTypeListLength<RangeTypes_>::Value;

			template < size_t Index_ >
			struct CallValid
			{
				static bool Call(const Tuple<RangeTypes_>& ranges)
				{ return ranges.template Get<Index_>().Valid(); }
			};

			template < size_t Index_ >
			struct CallFirst
			{
				static void Call(Tuple<RangeTypes_>& ranges)
				{ ranges.template Get<Index_>().First(); }
			};

			template < size_t Index_ >
			struct CallNext
			{
				static void Call(Tuple<RangeTypes_>& ranges)
				{ ranges.template Get<Index_>().Next(); }
			};

			template < size_t Index_ >
			struct CallPrev
			{
				static void Call(Tuple<RangeTypes_>& ranges)
				{ ranges.template Get<Index_>().Prev(); }
			};

			class ValuesGetter
			{
				template < typename Range_ >
				struct GetValueType
				{ typedef typename Range_::ValueType ValueT; };

			public:
				typedef typename TypeListTransform<RangeTypes_, GetValueType>::ValueT Types;

			private:
				Tuple<RangeTypes_>&	_ranges;

			public:
				explicit ValuesGetter(Tuple<RangeTypes_>& ranges) : _ranges(ranges) { }

				template < int Index_ >
				typename GetTypeListItem<Types, Index_>::ValueT Get() const
				{ return _ranges.template Get<Index_>().Get(); }
			};

		private:
			FuncType_							_func;
			Tuple<RangeTypes_>					_ranges;
			optional<typename base::ValueType>	_value;

		public:
			RangeZipper(const FuncType_& func, const Tuple<RangeTypes_>& ranges) : _func(func), _ranges(ranges) { }

			bool Valid() const
			{ return ForIf<RangeCount, CallValid>::Do(_ranges); }

			typename base::ValueType Get()
			{
				if (!_value)
				{
					const ValuesGetter getter(_ranges);
					const Tuple<typename ValuesGetter::Types> values(TupleConstructorTag(), getter);

					_value.emplace(FunctorInvoker::Invoke(_func, values));
				}
				return *_value;
			}

			bool Equals(const RangeZipper& other) const
			{ return TupleEquals()(_ranges, other._ranges); }

			Self& First()
			{
				For<RangeCount, CallFirst>::Do(wrap_ref(_ranges));
				_value.reset();
				return *this;
			}

			Self& Next()
			{
				For<RangeCount, CallNext>::Do(wrap_ref(_ranges));
				_value.reset();
				return *this;
			}

			Self& Prev()
			{
				For<RangeCount, CallPrev>::Do(wrap_ref(_ranges));
				_value.reset();
				return *this;
			}

			Self& Last()
			{
				First();
				bool empty = true;
				while (Valid())
				{
					Next();
					empty = false;
				}

				if (!empty)
					Prev();

				_value.reset();
				return *this;
			}
		};


		template < typename SrcRange_, typename DstRange_ >
		typename EnableIf<IsRange<DstRange_>::Value, DstRange_>::ValueT Copy(SrcRange_ src, DstRange_ dst)
		{
			for (; src.Valid(); src.Next(), dst.Next())
			{
				STINGRAYKIT_CHECK(dst.Valid(), "Destination range is not valid!");
				dst.Get() = src.Get();
			}
			return dst;
		}


		template < typename SrcRange_, typename DstIterator_ >
		typename EnableIf<!IsRange<DstIterator_>::Value, DstIterator_>::ValueT Copy(SrcRange_ src, DstIterator_ dst)
		{
			for (; src.Valid(); src.Next(), ++dst)
				*dst = src.Get();
			return dst;
		}


		template < typename Range_ >
		typename Range_::ValueType First(const Range_& range)
		{
			STINGRAYKIT_CHECK(range.Valid(), "Range is not valid!");
			return range.Get();
		}


		template < typename Range_ >
		typename Decay<typename Range_::ValueType>::ValueT FirstOrDefault(Range_ range)
		{ return range.Valid() ? range.Get() : typename Decay<typename Range_::ValueType>::ValueT(); }


		template < typename SrcRange_, typename Predicate_ >
		RangeFilter<SrcRange_, Predicate_> Filter(const SrcRange_& src, const Predicate_& predicate)
		{ return RangeFilter<SrcRange_, Predicate_>(src, predicate); }


		template < typename Dst_, typename SrcRange_ >
		RangeOfType<Dst_, SrcRange_> OfType(const SrcRange_& src)
		{ return RangeOfType<Dst_, SrcRange_>(src); }


		template < typename SrcRange_ >
		RangeReverser<SrcRange_> Reverse(const SrcRange_& src)
		{ return RangeReverser<SrcRange_>(src); }


		template < typename SrcRange_, typename Functor_ >
		RangeTransformer<SrcRange_, Functor_> Transform(const SrcRange_& src, const Functor_& functor)
		{ return RangeTransformer<SrcRange_, Functor_>(src, functor); }


		template < typename Range_ >
		RangeTaker<Range_> Take(const Range_& range, size_t count)
		{ return RangeTaker<Range_>(range, count); }


		namespace Detail
		{
			template < typename Src_ >
			struct MapKeysFunctor
			{
				typedef typename RemoveReference<Src_>::ValueT::first_type RetType;
				RetType operator() (const Src_& src) const
				{ return src.first; }
			};

			template < typename Src_ >
			struct MapValuesFunctor
			{
				typedef typename RemoveReference<Src_>::ValueT::second_type RetType;
				RetType operator() (const Src_& src) const
				{ return src.second; }
			};
		}


		template < typename SrcRange_ >
		RangeTransformer<SrcRange_, Detail::MapKeysFunctor<typename SrcRange_::ValueType> > MapKeys(const SrcRange_& src)
		{ return RangeTransformer<SrcRange_, Detail::MapKeysFunctor<typename SrcRange_::ValueType> >(src, Detail::MapKeysFunctor<typename SrcRange_::ValueType>()); }


		template < typename SrcRange_ >
		RangeTransformer<SrcRange_, Detail::MapValuesFunctor<typename SrcRange_::ValueType> > MapValues(const SrcRange_& src)
		{ return RangeTransformer<SrcRange_, Detail::MapValuesFunctor<typename SrcRange_::ValueType> >(src, Detail::MapValuesFunctor<typename SrcRange_::ValueType>()); }


		template < typename SrcRange_ >
		RangeCycler<SrcRange_> Cycle(const SrcRange_& src)
		{ return RangeCycler<SrcRange_>(src); }


		template < typename Range_, typename Functor_ >
		void ForEach(Range_ range, const Functor_& functor)
		{
			for (; range.Valid(); range.Next())
				FunctorInvoker::InvokeArgs(functor, range.Get());
		}


		template < typename Range_ >
		size_t Count(Range_ range)
		{
			size_t result = 0;
			for (; range.Valid(); ++result, range.Next())
				;
			return result;
		}


		template < typename Range_, class Value_ >
		bool Contains(Range_ range, Value_ value)
		{
			for (; range.Valid(); range.Next())
				if (range.Get() == value)
					return true;
			return false;
		}


		template < typename Range_, class Value_ >
		optional<size_t> IndexOf(Range_ range, Value_ value)
		{
			size_t result = 0;
			for (; range.Valid(); range.Next())
			{
				if (range.Get() == value)
					return result;
				++result;
			}
			return null;
		}


		template < typename Range_, class Predicate_ >
		bool AnyOf(Range_ range, Predicate_ predicate)
		{
			for (; range.Valid(); range.Next())
				if (FunctorInvoker::InvokeArgs(predicate, range.Get()))
					return true;
			return false;
		}


		template < typename Range_, class Predicate_ >
		bool AllOf(Range_ range, Predicate_ predicate)
		{
			for (; range.Valid(); range.Next())
				if (!FunctorInvoker::InvokeArgs(predicate, range.Get()))
					return false;
			return true;
		}


		template < typename Range_, class Predicate_ >
		bool NoneOf(Range_ range, Predicate_ predicate)
		{
			for (; range.Valid(); range.Next())
				if (FunctorInvoker::InvokeArgs(predicate, range.Get()))
					return false;
			return true;
		}


		namespace Detail
		{
			template < typename Range_ >
			struct RangeToValue
			{
				typedef typename If<Range_::ReturnsTemporary, typename Decay<typename Range_::ValueType>::ValueT, typename Range_::ValueType>::ValueT ValueT;
			};
		}


		template < typename Range_ >
		typename Detail::RangeToValue<Range_>::ValueT ElementAt(Range_ range, size_t index)
		{
			size_t current = 0;
			for (; range.Valid(); range.Next(), ++current)
				if (index == current)
					return range.Get();
			STINGRAYKIT_THROW(IndexOutOfRangeException(index, current));
		}


		template < typename Range_ >
		typename Decay<typename Range_::ValueType>::ValueT ElementAtOrDefault(Range_ range, size_t index)
		{
			size_t current = 0;
			for (; range.Valid(); range.Next(), ++current)
				if (index == current)
					return range.Get();
			return typename Decay<typename Range_::ValueType>::ValueT();
		}


		template < typename Range_ >
		typename Decay<typename Range_::ValueType>::ValueT Sum(Range_ range)
		{
			typename Decay<typename Range_::ValueType>::ValueT result = 0;
			for (; range.Valid(); range.Next())
				result += range.Get();
			return result;
		}


		template < typename Range_, class Comparer_ >
		optional<typename Detail::RangeToValue<Range_>::ValueT> MinElement(Range_ range, Comparer_ comparer)
		{
			optional<typename Detail::RangeToValue<Range_>::ValueT> result;
			for (; range.Valid(); range.Next())
				if (!result || comparer(range.Get(), *result))
					result = range.Get();
			return result;
		}


		template < typename Range_ >
		optional<typename Detail::RangeToValue<Range_>::ValueT> MinElement(Range_ range)
		{ return MinElement(range, comparers::Less()); }


		template < typename Range_, class Comparer_ >
		optional<typename Detail::RangeToValue<Range_>::ValueT> MaxElement(Range_ range, Comparer_ comparer)
		{
			optional<typename Detail::RangeToValue<Range_>::ValueT> result;
			for (; range.Valid(); range.Next())
				if (!result || comparer(*result, range.Get()))
					result = range.Get();
			return result;
		}


		template < typename Range_ >
		optional<typename Detail::RangeToValue<Range_>::ValueT> MaxElement(Range_ range)
		{ return MaxElement(range, comparers::Less()); }


		template < typename Range_, typename Functor_ >
		optional<typename Detail::RangeToValue<Range_>::ValueT> Fold(Range_ range, const Functor_& functor)
		{
			optional<typename Detail::RangeToValue<Range_>::ValueT> result;
			for (; range.Valid(); range.Next())
				result = result ? FunctorInvoker::InvokeArgs(functor, *result, range.Get()) : range.Get();
			return result;
		}


		template < typename Result_, typename Range_, typename Functor_ >
		Result_ Fold(Range_ range, const Result_& initialValue, const Functor_& functor)
		{
			Result_ result = initialValue;
			for (; range.Valid(); range.Next())
				result = FunctorInvoker::InvokeArgs(functor, result, range.Get());
			return result;
		}


		template < typename Collection_ >
		RangeSplitter<typename Collection_::const_iterator> Split(const Collection_& collection, size_t maxFragmentSize)
		{ return RangeSplitter<typename Collection_::const_iterator>(collection.begin(), collection.end(), maxFragmentSize); }


		template < typename It_ >
		RangeSplitter<It_> Split(const It_& begin, const It_& end, size_t maxFragmentSize)
		{ return RangeSplitter<It_>(begin, end, maxFragmentSize); }


		template < typename FuncType, typename... RangeTypes >
		RangeZipper<FuncType, TypeList<RangeTypes...>> Zip(const FuncType& func, const RangeTypes&... ranges)
		{ return RangeZipper<FuncType, TypeList<RangeTypes...>>(func, MakeTuple(ranges...)); }

	}


	namespace Detail
	{
		template < typename T, typename Enabler >
		struct ToRangeImpl
		{
			typedef typename If<IsConst<T>::Value, typename T::const_iterator, typename T::iterator>::ValueT IterType;
			typedef Range::IteratorRange<IterType> ValueT;

			static ValueT Do(T& collection)
			{ return ValueT(std::begin(collection), std::begin(collection), std::end(collection)); }
		};


		template < typename ArrayType_ >
		struct ToRangeImpl<ArrayType_, typename EnableIf<IsArray<ArrayType_>::Value, void>::ValueT>
		{
			typedef typename RemoveExtent<ArrayType_>::ValueT MemberType;
			typedef Range::IteratorRange<MemberType*> ValueT;

			static ValueT Do(ArrayType_& arr)
			{ return ValueT(std::begin(arr), std::begin(arr), std::end(arr)); }
		};


		template < typename IterType_ >
		struct ToRangeImpl<std::pair<IterType_, IterType_>, void>
		{
			typedef Range::IteratorRange<IterType_> ValueT;

			static ValueT Do(std::pair<IterType_, IterType_>& p)
			{ return ValueT(p.first, p.first, p.second); }
		};


		template < typename IterType_ >
		struct ToRangeImpl<const std::pair<IterType_, IterType_>, void>
		{
			typedef Range::IteratorRange<IterType_> ValueT;

			static ValueT Do(const std::pair<IterType_, IterType_>& p)
			{ return ValueT(p.first, p.first, p.second); }
		};


		template < typename Range_ >
		struct ToRangeImpl<Range_, typename EnableIf<IsRange<Range_>::Value, void>::ValueT>
		{
			typedef Range_ ValueT;

			static ValueT Do(const Range_& r)
			{ return r; }
		};
	}


	template < typename It_ >
	Range::IteratorRange<It_> ToRange(It_ begin, It_ end)
	{ return Range::IteratorRange<It_>(begin, begin, end); }


	template < typename Range_ >
	struct FirstTransformerImpl<Range_, typename EnableIf<IsRange<Range_>::Value, void>::ValueT>
	{
		typedef typename Range_::ValueType ValueT;

		static ValueT Do(const Range_& range)
		{ return Range::First(range); }
	};


	template < typename Range_ >
	struct FirstOrDefaultTransformerImpl<Range_, typename EnableIf<IsRange<Range_>::Value, void>::ValueT>
	{
		typedef typename Decay<typename Range_::ValueType>::ValueT ValueT;

		static ValueT Do(const Range_& range, const FirstOrDefaultTransformer& action)
		{ return Range::FirstOrDefault(range); }
	};


	template < typename Range_, typename Predicate_ >
	struct FilterTransformerImpl<Range_, Predicate_, typename EnableIf<IsRange<Range_>::Value, void>::ValueT>
	{
		typedef Range::RangeFilter<Range_, Predicate_> ValueT;

		static ValueT Do(const Range_& range, const FilterTransformer<Predicate_>& action)
		{ return ValueT(range, action.GetPredicate()); }
	};


	template < typename Range_ >
	struct ReverseTransformerImpl<Range_, typename EnableIf<IsRange<Range_>::Value, void>::ValueT>
	{
		typedef Range::RangeReverser<Range_> ValueT;

		static ValueT Do(const Range_& range, const ReverseTransformer& action)
		{ return ValueT(range); }
	};


	template < typename Range_, typename Functor_ >
	struct TransformTransformerImpl<Range_, Functor_, typename EnableIf<IsRange<Range_>::Value, void>::ValueT>
	{
		typedef Range::RangeTransformer<Range_, Functor_> ValueT;

		static ValueT Do(const Range_& range, const TransformTransformer<Functor_>& action)
		{ return ValueT(range, action.GetFunctor()); }
	};


	template < typename Range_, typename Dst_ >
	struct CastTransformerImpl<Range_, Dst_, typename EnableIf<IsRange<Range_>::Value, void>::ValueT>
	{
		typedef Range::RangeCaster<Dst_, Range_> ValueT;

		static ValueT Do(const Range_& range, const CastTransformer<Dst_>& action)
		{ return ValueT(range); }
	};


	template < typename Range_, typename Dst_ >
	struct OfTypeTransformerImpl<Range_, Dst_, typename EnableIf<IsRange<Range_>::Value, void>::ValueT>
	{
		typedef Range::RangeOfType<Dst_, Range_> ValueT;

		static ValueT Do(const Range_& range, const OfTypeTransformer<Dst_>& action)
		{ return ValueT(range); }
	};


	template < typename Range_ >
	struct AnyTransformerImpl<Range_, typename EnableIf<IsRange<Range_>::Value, void>::ValueT>
	{
		typedef bool ValueT;

		static ValueT Do(const Range_& range, const AnyTransformer& action)
		{ return range.Valid(); }
	};


	template < typename Range_ >
	struct CountTransformerImpl<Range_, typename EnableIf<IsRange<Range_>::Value, void>::ValueT>
	{
		typedef size_t ValueT;

		static ValueT Do(const Range_& range, const CountTransformer& action)
		{ return Range::Count(range); }
	};


	template < typename Range_ >
	struct TakeTransformerImpl<Range_, typename EnableIf<IsRange<Range_>::Value, void>::ValueT>
	{
		typedef Range::RangeTaker<Range_> ValueT;

		static ValueT Do(const Range_& range, const TakeTransformer& action)
		{ return Range::Take(range, action.GetCount()); }
	};

}

#endif
