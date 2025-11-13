#ifndef STINGRAYKIT_COLLECTION_RANGE_H
#define STINGRAYKIT_COLLECTION_RANGE_H

// Copyright (c) 2011 - 2025, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/collection/RangeBase.h>
#include <stingraykit/collection/ToRange.h>
#include <stingraykit/collection/Transformers.h>
#include <stingraykit/math.h>
#include <stingraykit/optional.h>
#include <stingraykit/RefStorage.h>

namespace stingray
{

	namespace Range
	{

		//template <typename ValueType_>
		//struct ForwardRange
		//{
		//    using ValueType = ValueType_;
		//
		//    bool Valid() const;
		//    ValueType Get() const;
		//
		//    bool Equals(const Self& other) const;
		//
		//    Self& First();
		//    Self& Next();
		//
		//    ValueType operator * () const                      { return Get(); }
		//    ArrowProxy<ValueType_> operator -> () const        { return Get(); }
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
			using ValueType = typename std::iterator_traits<Iterator_>::reference;

		private:
			Iterator_	_it;

		public:
			explicit OutputIteratorRange(const Iterator_& it) : _it(it) { }

			bool Valid() const			{ return true; }
			ValueType Get() const		{ return *_it; }

			void First()				{ static_assert(sizeof(Iterator_) < 0, "No first in output range"); }
			void Next()					{ ++_it; }
		};


		template < typename It_ >
		class IteratorRange : public RangeBase<IteratorRange<It_>, typename std::iterator_traits<It_>::reference, typename std::iterator_traits<It_>::iterator_category>
		{
			using Self = IteratorRange;

		private:
			It_		_begin;
			It_		_it;
			It_		_end;

		public:
			IteratorRange(const It_& begin, const It_& it, const It_& end) : _begin(begin), _it(it), _end(end) { }

			bool Valid() const
			{ return _it != _end; }

			typename Self::ValueType Get() const
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

			Self& End()
			{ _it = _end; return *this; }

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


		template < typename LhsValueType, typename RhsValueType >
		struct GetGeneralValueTypeHelper
		{
			using DereferencedLhsValueType = typename RemoveReference<LhsValueType>::ValueT;
			using DereferencedRhsValueType = typename RemoveReference<RhsValueType>::ValueT;

			static const bool DropReference = IsReference<LhsValueType>::Value != IsReference<RhsValueType>::Value
					|| !IsSame<DereferencedLhsValueType, DereferencedRhsValueType>::Value;

			using LhsValueT = typename If<DropReference, DereferencedLhsValueType, LhsValueType>::ValueT;
			using RhsValueT = typename If<DropReference, DereferencedRhsValueType, RhsValueType>::ValueT;

			using ValueT = typename If<IsConvertible<LhsValueT, RhsValueT>::Value, RhsValueT, LhsValueT>::ValueT;
		};


		template < typename LhsCategory, typename RhsCategory >
		struct GetGeneralCategoryHelper
		{ using ValueT = typename If<IsInherited<LhsCategory, RhsCategory>::Value, RhsCategory, LhsCategory>::ValueT; };


		template < typename Range_, typename Predicate_ >
		class RangeFilter : public RangeBase<RangeFilter<Range_, Predicate_>, typename Range_::ValueType, typename GetGeneralCategoryHelper<typename Range_::Category, std::bidirectional_iterator_tag>::ValueT>
		{
			using Self = RangeFilter;

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
			typename Self::ValueType Get() const			{ return _impl.Get(); }
			bool Equals(const RangeFilter& other) const		{ return _impl == other._impl; }
			Self& First()									{ _impl.First(); FindNext(); return *this; }
			Self& Next()									{ _impl.Next(); FindNext(); return *this; }
			Self& Last()									{ _impl.Last(); FindPrevOrInvalidate(); return *this; }
			Self& Prev()									{ _impl.Prev(); FindPrev(); return *this; }
			Self& End()										{ _impl.End(); return *this; }

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

			void FindPrevOrInvalidate()
			{
				const Range_ first(Range_(_impl).First());

				while (_impl.Valid() && !FunctorInvoker::InvokeArgs(_predicate, _impl.Get()))
				{
					if (_impl == first)
					{
						_impl.End();
						break;
					}
					else
						_impl.Prev();
				}
			}
		};


		template < typename Dst_, typename Range_ >
		class RangeCaster : public RangeBase<RangeCaster<Dst_, Range_>, Dst_, typename Range_::Category>
		{
			using Self = RangeCaster;

		public:
			static const bool ReturnsTemporary = Range_::ReturnsTemporary;

		private:
			Range_						_impl;
			mutable optional<Dst_>		_cache;

		public:
			explicit RangeCaster(const Range_& impl) : _impl(impl) { }

			bool Valid() const								{ return _impl.Valid(); }
			typename Self::ValueType Get() const			{ DoCast(); return *_cache; }
			bool Equals(const RangeCaster& other) const		{ return _impl == other._impl; }
			Self& First()									{ _impl.First(); _cache.reset(); return *this; }
			Self& Next()									{ _impl.Next(); _cache.reset(); return *this; }
			Self& Last()									{ _impl.Last(); _cache.reset(); return *this; }
			Self& Prev()									{ _impl.Prev(); _cache.reset(); return *this; }
			Self& End()										{ _impl.End(); _cache.reset(); return *this; }

			size_t GetPosition() const						{ return _impl.GetPosition(); }
			size_t GetSize() const							{ return _impl.GetSize(); }
			Self& Move(int distance)						{ _impl.Move(distance); _cache.reset(); return *this; }

		private:
			void DoCast() const
			{
				if (!_cache)
					_cache = STINGRAYKIT_CHECKED_DYNAMIC_CASTER(_impl.Get());
			}
		};


		template < typename Dst_, typename Range_ >
		class RangeOfType : public RangeBase<RangeOfType<Dst_, Range_>, Dst_, typename GetGeneralCategoryHelper<typename Range_::Category, std::bidirectional_iterator_tag>::ValueT>
		{
			using Self = RangeOfType;
			using Storage = RefStorage<Dst_>;

		public:
			static const bool ReturnsTemporary = Range_::ReturnsTemporary;

		private:
			Range_							_impl;
			typename Storage::ValueType		_dst;

		public:
			explicit RangeOfType(const Range_& impl) : _impl(impl) { FindNext(); }

			bool Valid() const								{ return _impl.Valid(); }
			typename Self::ValueType Get() const			{ return Storage::Unwrap(_dst); }
			bool Equals(const RangeOfType& other) const		{ return _impl == other._impl; }
			Self& First()									{ _impl.First(); FindNext(); return *this; }
			Self& Next()									{ _impl.Next(); FindNext(); return *this; }
			Self& Last()									{ _impl.Last(); FindPrevOrInvalidate(); return *this; }
			Self& Prev()									{ _impl.Prev(); FindPrev(); return *this; }
			Self& End()										{ _impl.End(); return *this; }

		private:
			void FindNext()
			{
				for ( ; _impl.Valid(); _impl.Next())
				{
					_dst = dynamic_caster(Storage::Wrap(_impl.Get()));
					if (_dst)
						return;
				}
			}

			void FindPrev()
			{
				for ( ; _impl.Valid(); _impl.Prev())
				{
					_dst = dynamic_caster(Storage::Wrap(_impl.Get()));
					if (_dst)
						return;
				}
			}

			void FindPrevOrInvalidate()
			{
				const Range_ first(Range_(_impl).First());

				for ( ; _impl.Valid(); _impl.Prev())
				{
					_dst = dynamic_caster(Storage::Wrap(_impl.Get()));
					if (_dst)
						break;

					if (_impl == first)
					{
						_impl.End();
						break;
					}
				}
			}
		};


		template < typename Range_ >
		class RangeReverser : public RangeBase<RangeReverser<Range_>, typename Range_::ValueType, typename Range_::Category>
		{
			using Self = RangeReverser;

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

			typename Self::ValueType Get() const
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

			Self& End()
			{
				_impl.First();
				_implBeforeBegin = true;
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
		class RangeTransformer : public RangeBase<RangeTransformer<Range_, Functor_>, typename AddConstLvalueReference<typename Decay<typename function_info<Functor_>::RetType>::ValueT>::ValueT, typename Range_::Category>
		{
			using Self = RangeTransformer;
			using RawValueType = typename Decay<typename function_info<Functor_>::RetType>::ValueT;

		public:
			static const bool ReturnsTemporary = true;

		private:
			Range_								_impl;
			Functor_							_functor;
			mutable optional<RawValueType>		_cache;

		public:
			RangeTransformer(const Range_& impl, const Functor_& functor)
				: _impl(impl), _functor(functor)
			{ }

			bool Valid() const									{ return _impl.Valid(); }
			typename Self::ValueType Get() const				{ DoTransform(); return *_cache; }
			bool Equals(const RangeTransformer& other) const	{ return _impl == other._impl; }
			Self& First()										{ _impl.First(); _cache.reset(); return *this; }
			Self& Next()										{ _impl.Next(); _cache.reset(); return *this; }
			Self& Last()										{ _impl.Last(); _cache.reset(); return *this; }
			Self& Prev()										{ _impl.Prev(); _cache.reset(); return *this; }
			Self& End()											{ _impl.End(); _cache.reset(); return *this; }

			size_t GetPosition() const							{ return _impl.GetPosition(); }
			size_t GetSize() const								{ return _impl.GetSize(); }
			Self& Move(int distance)							{ _impl.Move(distance); _cache.reset(); return *this; }

		private:
			void DoTransform() const
			{
				if (!_cache)
					_cache.emplace(FunctorInvoker::InvokeArgs(_functor, _impl.Get()));
			}
		};


		template < typename Range_ >
		class RangeDropper : public RangeBase<RangeDropper<Range_>, typename Range_::ValueType, typename Range_::Category>
		{
			using Self = RangeDropper;

		public:
			static const bool ReturnsTemporary = Range_::ReturnsTemporary;

		private:
			const Range_		_initial;
			optional<Range_>	_impl;

		public:
			RangeDropper(const Range_& impl, size_t count)
				: _initial(DoDrop(impl, count)), _impl(_initial)
			{ }

			bool Valid() const
			{ return _impl->Valid(); }

			typename Self::ValueType Get() const
			{ return _impl->Get(); }

			bool Equals(const RangeDropper& other) const
			{ return _initial == other._initial && _impl == other._impl; }

			Self& First()
			{
				_impl.emplace(_initial);
				return *this;
			}

			Self& Next()
			{
				_impl->Next();
				return *this;
			}

			Self& Last()
			{
				if (_initial.Valid())
					_impl->Last();
				else
					_impl.emplace(_initial);

				return *this;
			}

			Self& Prev()
			{
				STINGRAYKIT_CHECK(*_impl != _initial, "Prev() at first element");
				_impl->Prev();
				return *this;
			}

			Self& End()
			{
				_impl->End();
				return *this;
			}

			size_t GetPosition() const
			{ return _impl->GetPosition() - _initial.GetPosition(); }

			size_t GetSize() const
			{ return _impl->GetSize() - _initial.GetPosition(); }

			Self& Move(int distance)
			{
				STINGRAYKIT_CHECK(GetPosition() + distance <= GetSize(), IndexOutOfRangeException(GetPosition() + distance, GetSize()));
				_impl->Move(distance);
				return *this;
			}

		private:
			template < typename Category_ = typename Self::Category, typename EnableIf<IsInherited<Category_, std::random_access_iterator_tag>::Value, int>::ValueT = 0 >
			static Range_ DoDrop(const Range_& impl_, size_t count)
			{
				Range_ impl(impl_);
				impl.Move(std::min(count, impl.GetSize() - impl.GetPosition()));
				return impl;
			}

			template < typename Category_ = typename Self::Category, typename EnableIf<!IsInherited<Category_, std::random_access_iterator_tag>::Value, int>::ValueT = 0 >
			static Range_ DoDrop(const Range_& impl_, size_t count)
			{
				Range_ impl(impl_);
				for (size_t index = 0; index < count && impl.Valid(); ++index)
					impl.Next();
				return impl;
			}
		};


		template < typename Range_ >
		class RangeTaker : public RangeBase<RangeTaker<Range_>, typename Range_::ValueType, typename Range_::Category>
		{
			using Self = RangeTaker;

		public:
			static const bool ReturnsTemporary = Range_::ReturnsTemporary;

		private:
			const Range_		_initial;
			optional<Range_>	_impl;
			optional<size_t>	_index;
			const size_t		_count;

		public:
			RangeTaker(const Range_& impl, size_t count)
				: _initial(impl), _impl(impl), _index(0), _count(count)
			{ }

			bool Valid() const
			{ return _impl->Valid() && _index && _index < _count; }

			typename Self::ValueType Get() const
			{
				STINGRAYKIT_CHECK(Valid(), "Get() behind last element");
				return _impl->Get();
			}

			bool Equals(const RangeTaker& other) const
			{
				const bool valid = Valid();
				return _initial == other._initial && _count == other._count && valid == other.Valid() && (!valid || (_index == other._index && _impl == other._impl));
			}

			Self& First()
			{
				_impl.emplace(_initial);
				_index = 0;
				return *this;
			}

			Self& Next()
			{
				STINGRAYKIT_CHECK(Valid(), "Next() behind last element");
				_impl->Next();
				++*_index;
				return *this;
			}

			Self& Last()
			{
				DoLast();
				return *this;
			}

			Self& Prev()
			{
				STINGRAYKIT_CHECK(_index > 0 || (!_index && _impl != _initial), "Prev() at first element");

				if (!_index)
					Last();
				else
				{
					_impl->Prev();
					--*_index;
				}

				return *this;
			}

			size_t GetPosition() const
			{ return _index ? *_index : GetSize(); }

			size_t GetSize() const
			{ return std::min(_initial.GetSize() - _initial.GetPosition(), _count); }

			Self& Move(int distance)
			{
				const size_t size = GetSize();
				const size_t position = GetPosition();
				STINGRAYKIT_CHECK(position + distance <= size, IndexOutOfRangeException(position + distance, size));

				if (position + distance == size)
					End();
				else
				{
					_impl->Move(distance - (position == size && size == _count ? _initial.GetSize() - _initial.GetPosition() - _count : 0));
					_index = position + distance;
				}

				return *this;
			}

			Self& End()
			{
				_impl->End();
				_index.reset();
				return *this;
			}

		private:
			template < typename Category_ = typename Self::Category, typename EnableIf<IsInherited<Category_, std::random_access_iterator_tag>::Value, int>::ValueT = 0 >
			void DoLast()
			{
				const size_t size = GetSize();
				if (size == 0)
					return;

				const size_t position = GetPosition();
				_impl->Move(position < size ? size - position - 1 : size + _initial.GetPosition() - _impl->GetPosition() - 1);
				_index = size - 1;
			}

			template < typename Category_ = typename Self::Category, typename EnableIf<!IsInherited<Category_, std::random_access_iterator_tag>::Value, int>::ValueT = 0 >
			void DoLast()
			{
				First();
				Self prev(*this);

				while (Valid())
				{
					Next();
					if (Valid())
						prev.Next();
				}

				_impl = std::move(prev._impl);
				_index = prev._index;
			}
		};


		/// Doesn't support neither random-access nor equality check for now
		template < typename Range_ >
		class RangeCycler : public RangeBase<RangeCycler<Range_>, typename Range_::ValueType, typename GetGeneralCategoryHelper<typename Range_::Category, std::bidirectional_iterator_tag>::ValueT>
		{
			using Self = RangeCycler;

		private:
			Range_ _impl;

		public:
			explicit RangeCycler(const Range_& impl)
				: _impl(impl)
			{ STINGRAYKIT_CHECK(_impl.Valid(), "Can't cycle empty range!"); }

			bool Valid() const
			{ return true; }

			typename Self::ValueType Get() const
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

			Self& End()
			{
				First();
				return *this;
			}

		protected:
			void CheckValid() const
			{ STINGRAYKIT_CHECK(_impl.Valid(), LogicException("Something is terribly wrong with internal range!")); }
		};


		//FIXME: get rid of Prev and Last methods, make splitter work more efficient with lists
		template < typename It_ >
		class RangeSplitter : public RangeBase<RangeSplitter<It_>, IteratorRange<It_>, std::forward_iterator_tag>
		{
			using Self = RangeSplitter;

			using RawValueType = typename Decay<typename Self::ValueType>::ValueT;
			using DiffType = typename std::iterator_traits<It_>::difference_type;

		private:
			const It_							_begin;
			const It_							_end;
			const DiffType						_maxFragmentSize;
			It_									_it;
			mutable optional<RawValueType>		_value;

		public:
			RangeSplitter(const It_& begin, const It_& end, DiffType maxFragmentSize)
				: _begin(begin), _end(end), _maxFragmentSize(maxFragmentSize), _it(_begin)
			{ }

			bool Valid() const
			{ return _it != _end; }

			typename Self::ValueType Get() const
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

			Self& Last()
			{
				_it = std::next(_begin, AlignDown(std::distance(_begin, _end), _maxFragmentSize));
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

			Self& End()
			{
				_it = _end;
				_value.reset();
				return *this;
			}

		private:
			DiffType GetSize() const
			{ return std::min<DiffType>(_maxFragmentSize, std::distance(_it, _end)); }
		};


		template < typename Range_ >
		struct RangeFlattenerHelper
		{
			using SubRangeType = typename Detail::ToRangeImpl<typename RemoveReference<typename Range_::ValueType>::ValueT>::ValueT;
			using ValueType = typename SubRangeType::ValueType;

			using Category = typename GetGeneralCategoryHelper<typename GetGeneralCategoryHelper<typename Range_::Category, typename SubRangeType::Category>::ValueT, std::bidirectional_iterator_tag>::ValueT;
		};


		template < typename Range_ >
		class RangeFlattener : public RangeBase<RangeFlattener<Range_>, typename RangeFlattenerHelper<Range_>::ValueType, typename RangeFlattenerHelper<Range_>::Category>
		{
			using Self = RangeFlattener;

			using SubRangeType = typename RangeFlattenerHelper<Range_>::SubRangeType;

		private:
			Range_					_range;
			optional<SubRangeType>	_currentSubRange;

		public:
			explicit RangeFlattener(const Range_& range)
				: _range(range)
			{ _currentSubRange = FindNext(); }

			bool Valid() const
			{ return _currentSubRange.is_initialized(); }

			typename Self::ValueType Get() const
			{
				STINGRAYKIT_CHECK(Valid(), "Range is not valid!");
				return _currentSubRange->Get();
			}

			bool Equals(const RangeFlattener& other) const
			{ return _range == other._range && _currentSubRange == other._currentSubRange; }

			Self& First()
			{
				_range.First();
				_currentSubRange = FindNext();

				return *this;
			}

			Self& Next()
			{
				STINGRAYKIT_CHECK(Valid(), "Range is not valid!");

				_currentSubRange->Next();
				if (!_currentSubRange->Valid())
				{
					_range.Next();
					_currentSubRange = FindNext();
				}

				return *this;
			}

			Self& Last()
			{
				_range.Last();
				_currentSubRange = FindPrev();

				return *this;
			}

			Self& Prev()
			{
				STINGRAYKIT_CHECK(Valid(), "Range is not valid!");

				if (_currentSubRange != SubRangeType(*_currentSubRange).First())
					_currentSubRange->Prev();
				else if (_range != Range_(_range).First())
				{
					_range.Prev();
					_currentSubRange = FindPrev();
				}
				else
					_currentSubRange.reset();

				return *this;
			}

			Self& End()
			{
				_range.End();
				_currentSubRange.reset();
				return *this;
			}

		private:
			optional<SubRangeType> FindNext()
			{
				for (; _range.Valid(); _range.Next())
				{
					const auto subRange = ToRange(_range.Get());
					if (subRange.Valid())
						return subRange;
				}

				return null;
			}

			optional<SubRangeType> FindPrev()
			{
				if (!_range.Valid())
					return null;

				while (true)
				{
					const auto subRange = ToRange(_range.Get()).Last();
					if (subRange.Valid())
						return subRange;

					if (_range == Range_(_range).First())
						return null;

					_range.Prev();
				}
			}
		};


		template < typename FuncType_, typename RangeTypes_ >
		class RangeZipper : public RangeBase<RangeZipper<FuncType_, RangeTypes_>, typename function_info<FuncType_>::RetType, std::forward_iterator_tag>
		{
			using Self = RangeZipper;

			using RawValueType = typename Decay<typename Self::ValueType>::ValueT;

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
			struct CallEnd
			{
				static void Call(Tuple<RangeTypes_>& ranges)
				{ ranges.template Get<Index_>().End(); }
			};

			class ValuesGetter
			{
				template < typename Range_ >
				struct GetValueType
				{ using ValueT = typename Range_::ValueType; };

			public:
				using Types = typename TypeListTransform<RangeTypes_, GetValueType>::ValueT;

			private:
				const Tuple<RangeTypes_>&	_ranges;

			public:
				explicit ValuesGetter(const Tuple<RangeTypes_>& ranges) : _ranges(ranges) { }

				template < int Index_ >
				typename GetTypeListItem<Types, Index_>::ValueT Get() const
				{ return _ranges.template Get<Index_>().Get(); }
			};

		private:
			FuncType_							_func;
			Tuple<RangeTypes_>					_ranges;
			mutable optional<RawValueType>		_value;

		public:
			RangeZipper(const FuncType_& func, const Tuple<RangeTypes_>& ranges) : _func(func), _ranges(ranges) { }

			bool Valid() const
			{ return ForIf<RangeCount, CallValid>::Do(_ranges); }

			typename Self::ValueType Get() const
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

				if (!Valid())
					End();

				_value.reset();
				return *this;
			}

			Self& Next()
			{
				For<RangeCount, CallNext>::Do(wrap_ref(_ranges));

				if (!Valid())
					End();

				_value.reset();
				return *this;
			}

			Self& End()
			{
				For<RangeCount, CallEnd>::Do(wrap_ref(_ranges));
				_value.reset();
				return *this;
			}
		};


		template < typename RangeTypes >
		struct RangeConcaterValueTypeHelper
		{
			template < typename RangeType >
			struct ToValueType
			{ using ValueT = typename RangeType::ValueType; };

			using ValueT = typename TypeListAccumulate<typename TypeListTransform<RangeTypes, ToValueType>::ValueT, GetGeneralValueTypeHelper, typename RangeTypes::ValueT::ValueType>::ValueT;
		};


		template < typename RangeTypes >
		struct RangeConcaterCategoryHelper
		{
			template < typename RangeType >
			struct ToCategory
			{ using ValueT = typename RangeType::Category; };

			using ValueT = typename TypeListAccumulate<typename TypeListTransform<RangeTypes, ToCategory>::ValueT, GetGeneralCategoryHelper, std::bidirectional_iterator_tag>::ValueT;
		};


		template < typename RangeTypes >
		class RangeConcater : public RangeBase<RangeConcater<RangeTypes>, typename RangeConcaterValueTypeHelper<RangeTypes>::ValueT, typename RangeConcaterCategoryHelper<RangeTypes>::ValueT>
		{
			using Self = RangeConcater;

			using TupleType = Tuple<RangeTypes>;

			static const size_t RangeCount = GetTypeListLength<RangeTypes>::Value;

		private:
			struct CallValid
			{
				template < size_t Index >
				static bool Do(const Self& self)
				{ return self._ranges.template Get<Index>().Valid(); }

				static bool DoElse(const Self& self)
				{ return false; }
			};

			struct CallGet
			{
				template < size_t Index >
				static typename Self::ValueType Do(const Self& self)
				{ return self._ranges.template Get<Index>().Get(); }

				static typename Self::ValueType DoElse(const Self& self)
				{ STINGRAYKIT_THROW(LogicException("Get() behind last element")); }
			};

			struct CallFirst
			{
				template < size_t Index >
				static void Do(Self& self)
				{ self._ranges.template Get<Index>().First(); }

				static void DoElse(Self& self) { }
			};

			struct CallNext
			{
				template < size_t Index >
				static void Do(Self& self)
				{
					auto& range = self._ranges.template Get<Index>();
					range.Next();

					if (!range.Valid())
					{
						range.First();
						self.template FindNextRange<Index + 1>();
					}
				}

				static void DoElse(Self& self)
				{ STINGRAYKIT_THROW(LogicException("Next() behind last element")); }
			};

			struct CallPrev
			{
				template < size_t Index >
				static void Do(Self& self)
				{
					auto& range = self._ranges.template Get<Index>();
					auto copy = range;

					if (range != copy.First())
						range.Prev();
					else
						self.template FindPrevRange<Index>();
				}

				static void DoElse(Self& self)
				{ self.template FindPrevRange<RangeCount>(); }
			};

		private:
			TupleType				_ranges;
			size_t					_index;

		public:
			explicit RangeConcater(const Tuple<RangeTypes>& ranges)
				: _ranges(ranges), _index(0)
			{ FindNextRange(); }

			bool Valid() const
			{ return CallAction<CallValid>(*this); }

			typename Self::ValueType Get() const
			{
				STINGRAYKIT_CHECK(Valid(), "Get() behind last element");
				return CallAction<CallGet>(*this);
			}

			bool Equals(const RangeConcater& other) const
			{ return _index == other._index && TupleEquals()(_ranges, other._ranges); }

			Self& First()
			{
				CallAction<CallFirst>(*this);
				FindNextRange();
				return *this;
			}

			Self& Next()
			{
				STINGRAYKIT_CHECK(Valid(), "Next() behind last element");
				CallAction<CallNext>(*this);
				return *this;
			}

			Self& Last()
			{
				CallAction<CallFirst>(*this);
				FindPrevRange<RangeCount>();
				return *this;
			}

			Self& Prev()
			{
				STINGRAYKIT_CHECK(!Equals(Self(_ranges).First()), "Range is not valid!");
				CallAction<CallPrev>(*this);
				return *this;
			}

			Self& End()
			{
				CallAction<CallFirst>(*this);
				_index = RangeCount;
				return *this;
			}

		private:
			template < typename Action, typename Self_, size_t Index = 0, typename EnableIf<Index < RangeCount, int>::ValueT = 0 >
			static decltype(auto) CallAction(Self_& self)
			{ return self._index == Index ? Action::template Do<Index>(self) : CallAction<Action, Self_, Index + 1>(self); }

			template < typename Action, typename Self_, size_t Index, typename EnableIf<Index >= RangeCount, int>::ValueT = 0 >
			static decltype(auto) CallAction(Self_& self)
			{ return Action::DoElse(self); }

			template < size_t Index = 0, typename EnableIf<Index < RangeCount, int>::ValueT = 0 >
			void FindNextRange()
			{
				if (_ranges.template Get<Index>().Valid())
					_index = Index;
				else
					FindNextRange<Index + 1>();
			}

			template < size_t Index, typename EnableIf<Index >= RangeCount, int>::ValueT = 0 >
			void FindNextRange()
			{ _index = RangeCount; }

			template < size_t Index, typename EnableIf<(Index > 0), int>::ValueT = 0 >
			void FindPrevRange()
			{
				auto& range = _ranges.template Get<Index - 1>();
				if (range.Valid())
				{
					_index = Index - 1;
					range.Last();
				}
				else
					FindPrevRange<Index - 1>();
			}

			template < size_t Index, typename EnableIf<Index == 0, int>::ValueT = 0 >
			void FindPrevRange()
			{ _index = RangeCount; }
		};


		template < typename T >
		class RangeSingle : public RangeBase<RangeSingle<T>, const T&, std::random_access_iterator_tag>
		{
			using Self = RangeSingle;

		private:
			T		_value;
			bool	_valid;

		public:
			template < typename U = T, typename EnableIf<!IsSame<typename Decay<U>::ValueT, InPlaceType>::Value && IsConstructible<T, U>::Value, int>::ValueT = 0 >
			explicit RangeSingle(U&& value) : _value(std::forward<U>(value)), _valid(true) { }

			template < typename... Us, typename EnableIf<IsConstructible<T, Us...>::Value, int>::ValueT = 0 >
			explicit RangeSingle(InPlaceType, Us&&... args) : _value(std::forward<Us>(args)...), _valid(true) { }

			bool Valid() const
			{ return _valid; }

			typename Self::ValueType Get() const
			{
				STINGRAYKIT_CHECK(Valid(), "Get() behind last element");
				return _value;
			}

			bool Equals(const RangeSingle& other) const
			{ return _valid == other._valid && _value == other._value; }

			Self& First()
			{
				_valid = true;
				return *this;
			}

			Self& Next()
			{
				STINGRAYKIT_CHECK(Valid(), "Next() behind last element");
				_valid = false;
				return *this;
			}

			Self& Last()
			{
				_valid = true;
				return *this;
			}

			Self& Prev()
			{
				STINGRAYKIT_CHECK(!Valid(), "Prev() at first element");
				_valid = true;
				return *this;
			}

			Self& End()
			{
				_valid = false;
				return *this;
			}

			size_t GetPosition() const
			{ return _valid ? 0 : 1; }

			size_t GetSize() const
			{ return 1; }

			Self& Move(int distance)
			{
				STINGRAYKIT_CHECK(GetPosition() + distance <= GetSize(), IndexOutOfRangeException(GetPosition() + distance, GetSize()));
				_valid = GetPosition() + distance == 0;
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
			STINGRAYKIT_CHECK(range.Valid(), InvalidOperationException());
			return range.Get();
		}


		namespace Detail
		{
			template < typename Range_ >
			struct RangeToNullableValue
			{
				using DecayedValueT = typename Decay<typename Range_::ValueType>::ValueT;

				using ValueT = typename If<IsNullable<DecayedValueT>::Value, DecayedValueT, optional<DecayedValueT>>::ValueT;
			};
		}


		template < typename Range_ >
		typename Detail::RangeToNullableValue<Range_>::ValueT FirstOrDefault(const Range_& range)
		{
			if (range.Valid())
				return range.Get();
			return null;
		}


		template < typename Range_, class Value_, typename EnableIf<IsConvertible<Value_, typename Decay<typename Range_::ValueType>::ValueT>::Value, int>::ValueT = 0 >
		typename Decay<typename Range_::ValueType>::ValueT FirstOrDefault(const Range_& range, Value_ value)
		{ return range.Valid() ? range.Get() : value; }


		template < typename SrcRange_, typename Predicate_ >
		auto Filter(const SrcRange_& src, const Predicate_& predicate)
		{ return RangeFilter<SrcRange_, Predicate_>(src, predicate); }


		template < typename Dst_, typename SrcRange_ >
		auto OfType(const SrcRange_& src)
		{ return RangeOfType<Dst_, SrcRange_>(src); }


		template < typename SrcRange_ >
		auto Reverse(const SrcRange_& src)
		{ return RangeReverser<SrcRange_>(src); }


		template < typename SrcRange_, typename Functor_ >
		auto Transform(const SrcRange_& src, const Functor_& functor)
		{ return RangeTransformer<SrcRange_, Functor_>(src, functor); }


		template < typename Range_ >
		auto Drop(const Range_& range, size_t count)
		{ return RangeDropper<Range_>(range, count); }


		template < typename Range_ >
		auto Take(const Range_& range, size_t count)
		{ return RangeTaker<Range_>(range, count); }


		namespace Detail
		{
			template < typename Src_ >
			struct MapKeysFunctor
			{
				using RetType = typename RemoveReference<Src_>::ValueT::first_type;
				RetType operator() (const Src_& src) const
				{ return src.first; }
			};

			template < typename Src_ >
			struct MapValuesFunctor
			{
				using RetType = typename RemoveReference<Src_>::ValueT::second_type;
				RetType operator() (const Src_& src) const
				{ return src.second; }
			};
		}


		template < typename SrcRange_ >
		auto MapKeys(const SrcRange_& src)
		{ return RangeTransformer<SrcRange_, Detail::MapKeysFunctor<typename SrcRange_::ValueType>>(src, Detail::MapKeysFunctor<typename SrcRange_::ValueType>()); }


		template < typename SrcRange_ >
		auto MapValues(const SrcRange_& src)
		{ return RangeTransformer<SrcRange_, Detail::MapValuesFunctor<typename SrcRange_::ValueType>>(src, Detail::MapValuesFunctor<typename SrcRange_::ValueType>()); }


		template < typename SrcRange_ >
		auto Cycle(const SrcRange_& src)
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


		template < typename Range_, class Value_, typename EnableIf<IsConvertible<Value_, typename Decay<typename Range_::ValueType>::ValueT>::Value, int>::ValueT = 0 >
		bool Contains(Range_ range, Value_ value)
		{
			for (; range.Valid(); range.Next())
				if (range.Get() == value)
					return true;
			return false;
		}


		template < typename Range_, class Predicate_, typename EnableIf<!IsConvertible<Predicate_, typename Decay<typename Range_::ValueType>::ValueT>::Value, int>::ValueT = 0 >
		bool Contains(Range_ range, Predicate_ predicate)
		{
			for (; range.Valid(); range.Next())
				if (FunctorInvoker::InvokeArgs(predicate, range.Get()))
					return true;
			return false;
		}


		template < typename Range_, class Value_, typename EnableIf<IsConvertible<Value_, typename Decay<typename Range_::ValueType>::ValueT>::Value, int>::ValueT = 0 >
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


		template < typename Range_, class Predicate_, typename EnableIf<!IsConvertible<Predicate_, typename Decay<typename Range_::ValueType>::ValueT>::Value, int>::ValueT = 0 >
		optional<size_t> IndexOf(Range_ range, Predicate_ predicate)
		{
			size_t result = 0;
			for (; range.Valid(); range.Next())
			{
				if (FunctorInvoker::InvokeArgs(predicate, range.Get()))
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
				using ValueT = typename If<Range_::ReturnsTemporary, typename Decay<typename Range_::ValueType>::ValueT, typename Range_::ValueType>::ValueT;
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
		typename Detail::RangeToNullableValue<Range_>::ValueT ElementAtOrDefault(Range_ range, size_t index)
		{
			size_t current = 0;
			for (; range.Valid(); range.Next(), ++current)
				if (index == current)
					return range.Get();
			return null;
		}


		template < typename Range_, class Value_, typename EnableIf<IsConvertible<Value_, typename Decay<typename Range_::ValueType>::ValueT>::Value, int>::ValueT = 0 >
		typename Decay<typename Range_::ValueType>::ValueT ElementAtOrDefault(Range_ range, size_t index, Value_ value)
		{
			size_t current = 0;
			for (; range.Valid(); range.Next(), ++current)
				if (index == current)
					return range.Get();
			return value;
		}


		template < typename Range_ >
		auto Sum(Range_ range)
		{
			auto result = typename Decay<typename Range_::ValueType>::ValueT();
			for (; range.Valid(); range.Next())
				result += range.Get();
			return result;
		}


		template < typename Range_, class Comparer_ >
		auto MinElement(Range_ range, Comparer_ comparer)
		{
			optional<typename Decay<typename Range_::ValueType>::ValueT> result;
			for (; range.Valid(); range.Next())
				if (!result || comparer(range.Get(), *result))
					result = range.Get();
			return result;
		}


		template < typename Range_ >
		auto MinElement(Range_ range)
		{ return MinElement(range, comparers::Less()); }


		template < typename Range_, class Comparer_ >
		auto MaxElement(Range_ range, Comparer_ comparer)
		{
			optional<typename Decay<typename Range_::ValueType>::ValueT> result;
			for (; range.Valid(); range.Next())
				if (!result || comparer(*result, range.Get()))
					result = range.Get();
			return result;
		}


		template < typename Range_ >
		auto MaxElement(Range_ range)
		{ return MaxElement(range, comparers::Less()); }


		template < typename Range_, typename Functor_ >
		auto Fold(Range_ range, const Functor_& functor)
		{
			optional<typename Decay<typename Range_::ValueType>::ValueT> result;
			for (; range.Valid(); range.Next())
			{
				if (result)
					result = FunctorInvoker::InvokeArgs(functor, *result, range.Get());
				else
					result = range.Get();
			}

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
		auto Split(const Collection_& collection, size_t maxFragmentSize)
		{ return RangeSplitter<typename Collection_::const_iterator>(collection.begin(), collection.end(), maxFragmentSize); }


		template < typename It_ >
		auto Split(const It_& begin, const It_& end, size_t maxFragmentSize)
		{ return RangeSplitter<It_>(begin, end, maxFragmentSize); }


		template < typename Range_ >
		auto Flatten(const Range_& range)
		{ return RangeFlattener<Range_>(range); }


		template < typename FuncType, typename... RangeTypes >
		auto Zip(const FuncType& func, const RangeTypes&... ranges)
		{ return RangeZipper<FuncType, TypeList<RangeTypes...>>(func, MakeTuple(ranges...)); }


		template < typename Range0, typename... RangeTypes >
		auto Concat(const Range0& range0, const RangeTypes&... ranges)
		{ return RangeConcater<TypeList<Range0, RangeTypes...>>(MakeTuple(range0, ranges...)); }


		template < typename T >
		auto Single(T&& value)
		{ return RangeSingle<typename Decay<T>::ValueT>(std::forward<T>(value)); }

		template < typename T, typename... Us >
		auto Single(Us&&... args)
		{ return RangeSingle<T>(InPlace, std::forward<Us>(args)...); }

	}


	namespace Detail
	{

		template < typename T, typename Enabler >
		struct ToRangeImpl
		{
			using IterType = typename T::const_iterator;
			using ValueT = Range::IteratorRange<IterType>;

			static ValueT Do(const T& collection)
			{ return ValueT(std::begin(collection), std::begin(collection), std::end(collection)); }
		};

		template < typename T >
		struct ToRangeImpl<T, typename EnableIf<decltype(std::declval<typename T::iterator>(), TrueType())::Value && !IsRange<T>::Value, void>::ValueT>
		{
			using IterType = typename If<IsConst<T>::Value, typename T::const_iterator, typename T::iterator>::ValueT;
			using ValueT = Range::IteratorRange<IterType>;

			static ValueT Do(T& collection)
			{ return ValueT(std::begin(collection), std::begin(collection), std::end(collection)); }
		};

		template < typename ArrayType_ >
		struct ToRangeImpl<ArrayType_, typename EnableIf<IsArray<ArrayType_>::Value, void>::ValueT>
		{
			using MemberType = typename RemoveExtent<ArrayType_>::ValueT;
			using ValueT = Range::IteratorRange<MemberType*>;

			static ValueT Do(ArrayType_& arr)
			{ return ValueT(std::begin(arr), std::begin(arr), std::end(arr)); }
		};

		template < typename IterType_ >
		struct ToRangeImpl<std::pair<IterType_, IterType_>, void>
		{
			using ValueT = Range::IteratorRange<IterType_>;

			static ValueT Do(std::pair<IterType_, IterType_>& p)
			{ return ValueT(p.first, p.first, p.second); }
		};

		template < typename IterType_ >
		struct ToRangeImpl<const std::pair<IterType_, IterType_>, void>
		{
			using ValueT = Range::IteratorRange<IterType_>;

			static ValueT Do(const std::pair<IterType_, IterType_>& p)
			{ return ValueT(p.first, p.first, p.second); }
		};

		template < typename Range_ >
		struct ToRangeImpl<Range_, typename EnableIf<IsRange<Range_>::Value, void>::ValueT>
		{
			using ValueT = Range_;

			static ValueT Do(const Range_& r)
			{ return r; }
		};

	}


	template < typename It_ >
	auto ToRange(It_ begin, It_ end)
	{ return Range::IteratorRange<It_>(begin, begin, end); }


	template < typename Range_ >
	struct FirstTransformerImpl<Range_, typename EnableIf<IsRange<Range_>::Value, void>::ValueT>
	{
		using ValueT = typename Range_::ValueType;

		static ValueT Do(const Range_& range, const FirstTransformer& action)
		{ return Range::First(range); }
	};


	template < typename Range_ >
	struct FirstOrDefaultTransformerImpl<Range_, typename EnableIf<IsRange<Range_>::Value, void>::ValueT>
	{
		using ValueT = typename Range::Detail::RangeToNullableValue<Range_>::ValueT;

		static ValueT Do(const Range_& range, const FirstOrDefaultTransformer& action)
		{ return Range::FirstOrDefault(range); }
	};


	template < typename Range_, typename Predicate_ >
	struct FilterTransformerImpl<Range_, Predicate_, typename EnableIf<IsRange<Range_>::Value, void>::ValueT>
	{
		using ValueT = Range::RangeFilter<Range_, Predicate_>;

		static ValueT Do(const Range_& range, const FilterTransformer<Predicate_>& action)
		{ return ValueT(range, action.GetPredicate()); }
	};


	template < typename Range_ >
	struct ReverseTransformerImpl<Range_, typename EnableIf<IsRange<Range_>::Value, void>::ValueT>
	{
		using ValueT = Range::RangeReverser<Range_>;

		static ValueT Do(const Range_& range, const ReverseTransformer& action)
		{ return ValueT(range); }
	};


	template < typename Range_, typename Functor_ >
	struct TransformTransformerImpl<Range_, Functor_, typename EnableIf<IsRange<Range_>::Value, void>::ValueT>
	{
		using ValueT = Range::RangeTransformer<Range_, Functor_>;

		static ValueT Do(const Range_& range, const TransformTransformer<Functor_>& action)
		{ return ValueT(range, action.GetFunctor()); }
	};


	template < typename Range_, typename Dst_ >
	struct CastTransformerImpl<Range_, Dst_, typename EnableIf<IsRange<Range_>::Value, void>::ValueT>
	{
		using ValueT = Range::RangeCaster<Dst_, Range_>;

		static ValueT Do(const Range_& range, const CastTransformer<Dst_>& action)
		{ return ValueT(range); }
	};


	template < typename Range_, typename Dst_ >
	struct OfTypeTransformerImpl<Range_, Dst_, typename EnableIf<IsRange<Range_>::Value, void>::ValueT>
	{
		using ValueT = Range::RangeOfType<Dst_, Range_>;

		static ValueT Do(const Range_& range, const OfTypeTransformer<Dst_>& action)
		{ return ValueT(range); }
	};


	template < typename Range_ >
	struct AnyTransformerImpl<Range_, typename EnableIf<IsRange<Range_>::Value, void>::ValueT>
	{
		using ValueT = bool;

		static ValueT Do(const Range_& range, const AnyTransformer& action)
		{ return range.Valid(); }
	};


	template < typename Range_ >
	struct CountTransformerImpl<Range_, typename EnableIf<IsRange<Range_>::Value, void>::ValueT>
	{
		using ValueT = size_t;

		static ValueT Do(const Range_& range, const CountTransformer& action)
		{ return Range::Count(range); }
	};


	template < typename Range_ >
	struct DropTransformerImpl<Range_, typename EnableIf<IsRange<Range_>::Value, void>::ValueT>
	{
		using ValueT = Range::RangeDropper<Range_>;

		static ValueT Do(const Range_& range, const DropTransformer& action)
		{ return Range::Drop(range, action.GetCount()); }
	};


	template < typename Range_ >
	struct TakeTransformerImpl<Range_, typename EnableIf<IsRange<Range_>::Value, void>::ValueT>
	{
		using ValueT = Range::RangeTaker<Range_>;

		static ValueT Do(const Range_& range, const TakeTransformer& action)
		{ return Range::Take(range, action.GetCount()); }
	};


	namespace Detail
	{

		template < typename Range_ >
		class StlRangeAdapter : public std::iterator<std::input_iterator_tag, typename Range_::value_type, typename Range_::difference_type, typename Range_::pointer, typename Range_::reference>
		{
		private:
			optional<Range_>		_range;

		public:
			StlRangeAdapter() // This means the end of a collection
			{ }

			StlRangeAdapter(const Range_& range)
				:	_range(range)
			{ }

			StlRangeAdapter& operator ++ ()
			{
				STINGRAYKIT_CHECK(_range, "Trying to increment an invalid range!");
				_range->Next();
				return *this;
			}

			typename Range_::value_type operator * () const
			{
				STINGRAYKIT_CHECK(_range, "Trying to dereference an invalid range!");
				return _range->Get();
			}

			bool operator != (const StlRangeAdapter& other) const
			{
				STINGRAYKIT_CHECK(!_range || !other._range, LogicException("Invalid comparison!"));
				return (_range && _range->Valid()) != (other._range && other._range->Valid());
			}

			bool operator == (const StlRangeAdapter& other) const
			{ return !(*this != other); }

			// whatever
		};

		template < typename Range_ >
		class RangeBasedForRangeAdapter
		{
		public:
			using const_iterator = StlRangeAdapter<Range_>;
			using value_type = typename Range_::ValueType;

		private:
			Range_		_range;

		public:
			explicit RangeBasedForRangeAdapter(const Range_& range) : _range(range) { }

			const_iterator begin() const	{ return StlRangeAdapter<Range_>(_range); }
			const_iterator end() const		{ return StlRangeAdapter<Range_>(); }
		};

	}


	template < typename Range_ >
	auto IterableRange(const Range_& range)
	{ return Detail::RangeBasedForRangeAdapter<Range_>(range); }

}

#endif
