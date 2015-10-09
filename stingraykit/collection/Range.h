#ifndef STINGRAYKIT_COLLECTION_RANGE_H
#define STINGRAYKIT_COLLECTION_RANGE_H

// Copyright (c) 2011 - 2015, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/RefStorage.h>
#include <stingraykit/collection/RangeBase.h>
#include <stingraykit/collection/ToRange.h>
#include <stingraykit/collection/Transformers.h>
#include <stingraykit/collection/iterators.h>
#include <stingraykit/dynamic_caster.h>
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
		//    ValueType operator [] (std::ptrdiff_t index) const  { CompileTimeAssert<false> errorNoBracketsOperator; (void)errorNoBracketsOperator; }
		//    Self& operator += (std::ptrdiff_t distance)         { Move(distance); return *this; }
		//    Self operator + (std::ptrdiff_t distance) const     { Self result(*this); return result += distance; }
		//    Self& operator -= (std::ptrdiff_t distance)         { Move(distance); return *this; }
		//    Self operator - (std::ptrdiff_t distance) const     { Self result(*this); return result -= distance; }
		//    std::ptrdiff_t operator - (const Self& other) const { return GetPosition() - other.GetPosition(); }
		//};


		template <typename Iterator_>
		class OutputIteratorRange
		{
		public:
			typedef typename std::iterator_traits<Iterator_>::reference ValueType;

		private:
			Iterator_ _it;

		public:
			OutputIteratorRange(const Iterator_& it) : _it(it)
			{ }

			bool Valid() const   { return true; }
			ValueType Get()      { return *_it; }

			void First()         { CompileTimeAssert<sizeof(Iterator_) < 0> errorNoFirstInOutputRange; (void)errorNoFirstInOutputRange; }
			void Next()          { ++_it; }
		};


		template <typename It_>
		class IteratorRange :
			public RangeBase<IteratorRange<It_>, typename std::iterator_traits<It_>::reference, typename std::iterator_traits<It_>::iterator_category>
		{
			typedef IteratorRange<It_> Self;
			typedef RangeBase<IteratorRange<It_>, typename std::iterator_traits<It_>::reference, typename std::iterator_traits<It_>::iterator_category> base;

		private:
			const It_     _begin;
			optional<It_> _it;
			const It_     _end;

		public:
			IteratorRange(const It_& begin, const It_& it, const It_& end) : _begin(begin), _it(it), _end(end)
			{ }

			bool Valid() const
			{ return _it && *_it != _end; }

			typename base::ValueType Get()
			{ return **_it; }

			bool Equals(const IteratorRange& other) const
			{ return _begin == other._begin && _it == other._it && _end == other._end; }

			Self& First()
			{ _it = _begin; return *this; }

			Self& Next()
			{
				if (!_it)
					_it = _begin;
				else
					++*_it;
				return *this;
			}

			Self& Last()
			{
				if (_begin != _end)
				{
					_it = _end;
					--*_it;
				}
				else
					_it.reset();
				return *this;
			}

			Self& Prev()
			{
				if (_it == _begin)
					_it.reset();
				else
					--*_it;
				return *this;
			}

			int GetPosition() const
			{ return _it ? *_it - _begin : -1; }

			size_t GetSize() const
			{ return _end - _begin; }

			Self& Move(int distance)
			{
				_it = _it ? *_it + distance : _begin + distance - 1;
				return *this;
			}
		};


		/// @brief Since filtering items removes random-access property from range, we need to appropriately change category
		template <typename Category_>
		struct RangeFilterCategoryHelper
		{ typedef Category_ ValueT; };


		template <>
		struct RangeFilterCategoryHelper<std::random_access_iterator_tag>
		{ typedef std::bidirectional_iterator_tag ValueT; };


		template <typename Range_, typename Predicate_>
		class RangeFilter : public RangeBase<RangeFilter<Range_, Predicate_>, typename Range_::ValueType, typename RangeFilterCategoryHelper<typename Range_::Category>::ValueT>
		{
			typedef RangeBase<RangeFilter<Range_, Predicate_>, typename Range_::ValueType, typename RangeFilterCategoryHelper<typename Range_::Category>::ValueT> base;
			typedef RangeFilter<Range_, Predicate_> Self;

		private:
			Range_     _impl;
			Predicate_ _predicate;

		public:
			RangeFilter(const Range_& impl, const Predicate_& predicate) : _impl(impl), _predicate(predicate)
			{ FindNext(); }

			bool Valid() const                          { return _impl.Valid(); }
			typename base::ValueType Get()              { return _impl.Get(); }
			bool Equals(const RangeFilter& other) const { return _impl == other._impl; }
			Self& First()                               { _impl.First(); FindNext(); return *this; }
			Self& Last()                                { _impl.Last(); FindPrev(); return *this; }
			Self& Next()                                { _impl.Next(); FindNext(); return *this; }
			Self& Prev()                                { _impl.Prev(); FindPrev(); return *this; }

		private:
			void FindNext()
			{
				while (_impl.Valid() && !_predicate(_impl.Get()))
					_impl.Next();
			}

			void FindPrev()
			{
				while (_impl.Valid() && !_predicate(_impl.Get()))
					_impl.Prev();
			}
		};


		template <typename Dst_, typename Range_>
		class RangeCaster : public RangeBase<RangeCaster<Dst_, Range_>, Dst_, typename Range_::Category>
		{
			typedef RangeCaster<Dst_, Range_> Self;
			typedef RangeBase<RangeCaster<Dst_, Range_>, Dst_, typename Range_::Category> base;

		private:
			Range_         _impl;
			optional<Dst_> _cache;

		public:
			RangeCaster(const Range_& impl) : _impl(impl)
			{ }

			bool Valid() const                          { return _impl.Valid(); }
			typename base::ValueType Get()              { DoCast(); return *_cache; }
			bool Equals(const RangeCaster& other) const { return _impl == other._impl; }
			Self& First()                               { _impl.First(); _cache.reset(); return *this; }
			Self& Next()                                { _impl.Next(); _cache.reset(); return *this; }
			Self& Last()                                { _impl.Last(); _cache.reset(); return *this; }
			Self& Prev()                                { _impl.Prev(); _cache.reset(); return *this; }

			int GetPosition() const                     { return _impl.GetPosition(); }
			size_t GetSize() const                      { return _impl.GetSize(); }
			Self& Move(int distance)                    { _impl.Move(distance); _cache.reset(); return *this; }

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


		template <typename Dst_, typename Range_>
		class RangeOfType : public RangeBase<RangeOfType<Dst_, Range_>, Dst_, typename RangeFilterCategoryHelper<typename Range_::Category>::ValueT>
		{
			typedef RangeOfType<Dst_, Range_> Self;
			typedef RangeBase<RangeOfType<Dst_, Range_>, Dst_, typename RangeFilterCategoryHelper<typename Range_::Category>::ValueT> base;
			typedef RefStorage<Dst_> Storage;

		private:
			Range_                      _impl;
			typename Storage::ValueType _dst;

		public:
			RangeOfType(const Range_& impl) : _impl(impl)
			{ FindNext(); }

			bool Valid() const                          { return _impl.Valid(); }
			typename base::ValueType Get()              { return Storage::Unwrap(_dst); }
			bool Equals(const RangeOfType& other) const { return _impl == other._impl; }
			Self& First()                               { _impl.First(); FindNext(); return *this; }
			Self& Next()                                { _impl.Next(); FindNext(); return *this; }
			Self& Last()                                { _impl.Last(); FindPrev(); return *this; }
			Self& Prev()                                { _impl.Prev(); FindPrev(); return *this; }

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


		template <typename Range_>
		class RangeReverser : public RangeBase<RangeReverser<Range_>, typename Range_::ValueType, typename Range_::Category>
		{
			typedef RangeReverser<Range_> Self;
			typedef RangeBase<RangeReverser<Range_>, typename Range_::ValueType, typename Range_::Category> base;

		private:
			Range_ _impl;

		public:
			RangeReverser(const Range_& impl) : _impl(impl)
			{ First(); }

			bool Valid() const                            { return _impl.Valid(); }
			typename base::ValueType Get()                { return _impl.Get(); }
			bool Equals(const RangeReverser& other) const { return _impl == other._impl; }
			Self& First()                                 { _impl.Last(); return *this; }
			Self& Next()                                  { _impl.Prev(); return *this; }
			Self& Last()                                  { _impl.First(); return *this; }
			Self& Prev()                                  { _impl.Next(); return *this; }

			int GetPosition() const                       { return static_cast<int>(_impl.GetSize()) - static_cast<int>(_impl.GetPosition()) - 1; }
			size_t GetSize() const                        { return _impl.GetSize(); }
			Self& Move(int distance)                      { _impl.Move(distance); return *this; }
		};


		template <typename Range_, typename Functor_>
		class RangeTransformer : public RangeBase<RangeTransformer<Range_, Functor_>, typename GetConstReferenceType<typename function_info<Functor_>::RetType>::ValueT, typename Range_::Category>
		{
			typedef RangeTransformer<Range_, Functor_> Self;
			typedef RangeBase<RangeTransformer<Range_, Functor_>, typename GetConstReferenceType<typename function_info<Functor_>::RetType>::ValueT, typename Range_::Category> base;

		private:
			Range_                                              _impl;
			Functor_                                            _functor;
			optional<typename function_info<Functor_>::RetType> _cache;

		public:
			RangeTransformer(const Range_& impl, const Functor_& functor) : _impl(impl), _functor(functor)
			{ }

			bool Valid() const                               { return _impl.Valid(); }
			typename base::ValueType Get()                   { DoTransform(); return *_cache; }
			bool Equals(const RangeTransformer& other) const { return _impl == other._impl; }
			Self& First()                                    { _impl.First(); _cache.reset(); return *this; }
			Self& Next()                                     { _impl.Next(); _cache.reset(); return *this; }
			Self& Last()                                     { _impl.Last(); _cache.reset(); return *this; }
			Self& Prev()                                     { _impl.Prev(); _cache.reset(); return *this; }

			int GetPosition() const                          { return _impl.GetPosition(); }
			size_t GetSize() const                           { return _impl.GetSize(); }
			Self& Move(int distance)                         { _impl.Move(distance); _cache.reset(); return *this; }

		private:
			void DoTransform()
			{
				if (!_cache)
					_cache = _functor(_impl.Get());
			}
		};


		/// Doesn't support neither random-access nor equality check for now
		template <typename Range_>
		class RangeCycler : public RangeBase<RangeCycler<Range_>, typename Range_::ValueType, typename RangeFilterCategoryHelper<typename Range_::Category>::ValueT>
		{
			typedef RangeCycler<Range_> Self;
			typedef RangeBase<RangeCycler<Range_>, typename Range_::ValueType, typename RangeFilterCategoryHelper<typename Range_::Category>::ValueT> base;

		private:
			Range_ _impl;

		public:
			RangeCycler(const Range_& impl) : _impl(impl)
			{ STINGRAYKIT_CHECK(_impl.Valid(), "Can't cycle empty range!"); }

			bool Valid() const             { return true; }
			typename base::ValueType Get() { return _impl.Get(); }

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
				_impl.Prev();
				if (_impl.Valid())
					return *this;

				_impl.Last();
				CheckValid();
				return *this;
			}

		protected:
			void CheckValid() const
			{ STINGRAYKIT_CHECK(_impl.Valid(), LogicException("Something is terribly wrong with internal range!")); }
		};


		template <typename SrcRange_, typename DstRange_>
		typename EnableIf<IsRange<DstRange_>::Value, DstRange_>::ValueT Copy(SrcRange_ src, DstRange_ dst)
		{
			for (; src.Valid(); src.Next(), dst.Next())
			{
				STINGRAYKIT_CHECK(dst.Valid(), "Destination range is not valid!");
				dst.Get() = src.Get();
			}
			return dst;
		}


		template <typename SrcRange_, typename DstIterator_>
		typename EnableIf<!IsRange<DstIterator_>::Value, DstIterator_>::ValueT Copy(SrcRange_ src, DstIterator_ dst)
		{
			for (; src.Valid(); src.Next(), ++dst)
				*dst = src.Get();
			return dst;
		}


		template <typename SrcRange_, typename Predicate_>
		RangeFilter<SrcRange_, Predicate_> Filter(const SrcRange_& src, const Predicate_& predicate)
		{ return RangeFilter<SrcRange_, Predicate_>(src, predicate); }


		template <typename Dst_, typename SrcRange_>
		RangeOfType<Dst_, SrcRange_> OfType(const SrcRange_& src)
		{ return RangeOfType<Dst_, SrcRange_>(src); }


		template <typename SrcRange_>
		RangeReverser<SrcRange_> Reverse(const SrcRange_& src)
		{ return RangeReverser<SrcRange_>(src); }


		template <typename SrcRange_, typename Functor_>
		RangeTransformer<SrcRange_, Functor_> Transform(const SrcRange_& src, const Functor_& functor)
		{ return RangeTransformer<SrcRange_, Functor_>(src, functor); }


		namespace Detail
		{
			template <typename Src_>
			struct MapKeysFunctor
			{
				typedef typename Dereference<Src_>::ValueT::first_type RetType;
				RetType operator() (const Src_& src) const
				{ return src.first; }
			};

			template <typename Src_>
			struct MapValuesFunctor
			{
				typedef typename Dereference<Src_>::ValueT::second_type RetType;
				RetType operator() (const Src_& src) const
				{ return src.second; }
			};
		}


		template <typename SrcRange_>
		RangeTransformer<SrcRange_, Detail::MapKeysFunctor<typename SrcRange_::ValueType> > MapKeys(const SrcRange_& src)
		{ return RangeTransformer<SrcRange_, Detail::MapKeysFunctor<typename SrcRange_::ValueType> >(src, Detail::MapKeysFunctor<typename SrcRange_::ValueType>()); }


		template <typename SrcRange_>
		RangeTransformer<SrcRange_, Detail::MapValuesFunctor<typename SrcRange_::ValueType> > MapValues(const SrcRange_& src)
		{ return RangeTransformer<SrcRange_, Detail::MapValuesFunctor<typename SrcRange_::ValueType> >(src, Detail::MapValuesFunctor<typename SrcRange_::ValueType>()); }


		template <typename SrcRange_>
		RangeCycler<SrcRange_> Cycle(const SrcRange_& src)
		{ return RangeCycler<SrcRange_>(src); }


		template <typename Range_, typename Functor_>
		void ForEach(Range_ range, const Functor_& functor)
		{
			for (; range.Valid(); range.Next())
				functor(range.Get());
		}


		template <typename Range_>
		size_t Count(Range_ range)
		{
			size_t result = 0;
			for (; range.Valid(); ++result, range.Next())
				;
			return result;
		}


		template <typename Range_, class Value_>
		bool Contains(Range_ range, Value_ value)
		{
			for (; range.Valid(); range.Next())
				if (range.Get() == value)
					return true;
			return false;
		}


		template <typename Range_, class Predicate_>
		bool AnyOf(Range_ range, Predicate_ predicate)
		{
			for (; range.Valid(); range.Next())
				if (predicate(range.Get()))
					return true;
			return false;
		}


		template <typename Range_, class Predicate_>
		bool AllOf(Range_ range, Predicate_ predicate)
		{
			for (; range.Valid(); range.Next())
				if (!predicate(range.Get()))
					return false;
			return true;
		}


		template <typename Range_, class Predicate_>
		bool NoneOf(Range_ range, Predicate_ predicate)
		{
			for (; range.Valid(); range.Next())
				if (predicate(range.Get()))
					return false;
			return true;
		}
	}


	namespace Detail
	{
		template <typename T, typename Enabler>
		struct ToRangeImpl
		{
			typedef typename If<IsConst<T>::Value, typename T::const_iterator, typename T::iterator>::ValueT IterType;
			typedef Range::IteratorRange<IterType> ValueT;

			static ValueT Do(T& collection)
			{ return ValueT(begin(collection), begin(collection), end(collection)); }
		};


		template <typename ArrayType_>
		struct ToRangeImpl<ArrayType_, typename EnableIf<IsArray<ArrayType_>::Value, void>::ValueT>
		{
			typedef typename RemoveExtent<ArrayType_>::ValueT MemberType;
			typedef Range::IteratorRange<MemberType*> ValueT;

			static ValueT Do(ArrayType_& arr)
			{ return ValueT(begin(arr), begin(arr), end(arr)); }
		};


		template <typename IterType_>
		struct ToRangeImpl<std::pair<IterType_, IterType_>, void>
		{
			typedef Range::IteratorRange<IterType_> ValueT;

			static ValueT Do(std::pair<IterType_, IterType_>& p)
			{ return ValueT(p.first, p.first, p.second); }
		};


		template <typename IterType_>
		struct ToRangeImpl<const std::pair<IterType_, IterType_>, void>
		{
			typedef Range::IteratorRange<IterType_> ValueT;

			static ValueT Do(const std::pair<IterType_, IterType_>& p)
			{ return ValueT(p.first, p.first, p.second); }
		};


		template <typename Range_>
		struct ToRangeImpl<Range_, typename EnableIf<IsRange<Range_>::Value, void>::ValueT>
		{
			typedef Range_ ValueT;

			static ValueT Do(const Range_& r)
			{ return r; }
		};
	}


	template <typename It_>
	Range::IteratorRange<It_> ToRange(It_ begin, It_ end)
	{ return Range::IteratorRange<It_>(begin, begin, end); }


	template <typename Range_, typename Predicate_>
	struct FilterTransformerImpl<Range_, Predicate_, typename EnableIf<IsRange<Range_>::Value, void>::ValueT>
	{
		typedef Range::RangeFilter<Range_, Predicate_> ValueT;

		static ValueT Do(const Range_& range, const FilterTransformer<Predicate_>& action)
		{ return ValueT(range, action.GetPredicate()); }
	};


	template <typename Range_>
	struct ReverseTransformerImpl<Range_, typename EnableIf<IsRange<Range_>::Value, void>::ValueT>
	{
		typedef Range::RangeReverser<Range_> ValueT;

		static ValueT Do(const Range_& range, const ReverseTransformer& action)
		{ return ValueT(range); }
	};


	template <typename Range_, typename Functor_>
	struct TransformTransformerImpl<Range_, Functor_, typename EnableIf<IsRange<Range_>::Value, void>::ValueT>
	{
		typedef Range::RangeTransformer<Range_, Functor_> ValueT;

		static ValueT Do(const Range_& range, const TransformTransformer<Functor_>& action)
		{ return ValueT(range, action.GetFunctor()); }
	};


	template <typename Range_, typename Dst_>
	struct CastTransformerImpl<Range_, Dst_, typename EnableIf<IsRange<Range_>::Value, void>::ValueT>
	{
		typedef Range::RangeCaster<Dst_, Range_> ValueT;

		static ValueT Do(const Range_& range, const CastTransformer<Dst_>& action)
		{ return ValueT(range); }
	};


	template <typename Range_, typename Dst_>
	struct OfTypeTransformerImpl<Range_, Dst_, typename EnableIf<IsRange<Range_>::Value, void>::ValueT>
	{
		typedef Range::RangeOfType<Dst_, Range_> ValueT;

		static ValueT Do(const Range_& range, const OfTypeTransformer<Dst_>& action)
		{ return ValueT(range); }
	};

}

#endif
