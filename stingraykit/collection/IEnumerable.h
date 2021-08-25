#ifndef STINGRAYKIT_COLLECTION_IENUMERABLE_H
#define STINGRAYKIT_COLLECTION_IENUMERABLE_H

// Copyright (c) 2011 - 2019, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/collection/IEnumerator.h>
#include <stingraykit/collection/RangeBase.h>
#include <stingraykit/collection/ToRange.h>

#define STINGRAYKIT_DECLARE_ENUMERABLE(ClassName) \
		using ClassName##Enumerable = stingray::IEnumerable<ClassName>; \
		STINGRAYKIT_DECLARE_PTR(ClassName##Enumerable); \
		STINGRAYKIT_DECLARE_ENUMERATOR(ClassName)

namespace stingray
{

	/**
	 * @addtogroup toolkit_collections
	 * @{
	 */

	template < typename T >
	struct IEnumerable
	{
		using ItemType = T;

		virtual ~IEnumerable() { }

		virtual shared_ptr<IEnumerator<ItemType>> GetEnumerator() const = 0;
	};


	template < typename T >
	struct IReversableEnumerable : public virtual IEnumerable<T>
	{
		using ItemType = T;

		~IReversableEnumerable() override { }

		virtual shared_ptr<IEnumerable<ItemType>> Reverse() const = 0;
	};


	template < typename T >
	struct IsEnumerable : IsInherited1ParamTemplate<T, IEnumerable> { };


	template < typename T >
	struct IsReversableEnumerable : IsInherited1ParamTemplate<T, IReversableEnumerable> { };


	namespace Detail
	{
		template < typename T >
		struct ToEnumeratorImpl<T, typename EnableIf<IsEnumerable<T>::Value, void>::ValueT>
		{
			using ValueT = IEnumerator<typename T::ItemType>;

			static shared_ptr<ValueT> Do(const shared_ptr<T>& src) { return STINGRAYKIT_REQUIRE_NOT_NULL(src)->GetEnumerator(); }
		};


		template < typename Enumerable_ >
		class EnumerableToRange : public Range::RangeBase<EnumerableToRange<Enumerable_>, typename Enumerable_::ItemType, std::forward_iterator_tag>
		{
			STINGRAYKIT_NONASSIGNABLE(EnumerableToRange);

		private:
			using base = Range::RangeBase<EnumerableToRange<Enumerable_>, typename Enumerable_::ItemType, std::forward_iterator_tag>;
			using Self = EnumerableToRange<Enumerable_>;

			using EnumeratorPtr = shared_ptr<IEnumerator<typename Enumerable_::ItemType>>;

		private:
			const Enumerable_&				_enumerable;
			EnumeratorPtr					_enumerator;
			size_t							_index;

		public:
			EnumerableToRange(const Enumerable_& enumerable) : _enumerable(enumerable)
			{ First(); }

			EnumerableToRange(const EnumerableToRange& range) : _enumerable(range._enumerable)
			{
				First();
				while (_index != range._index)
					Next();
			}

			EnumerableToRange(EnumerableToRange&& range) = default;

			bool Valid() const							{ return _enumerator->Valid(); }
			typename base::ValueType Get() const		{ return _enumerator->Get(); }

			bool Equals(const Self& other) const
			{ return &_enumerable == &other._enumerable && _index == other._index; }

			Self& First()
			{
				_enumerator = _enumerable.GetEnumerator();
				_index = 0;
				return *this;
			}

			Self& Next()
			{
				_enumerator->Next();
				++_index;
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

				_enumerator = prev._enumerator;
				_index = prev._index;
				return *this;
			}
		};


		template < typename Enumerable_ >
		struct ToRangeImpl<Enumerable_, typename EnableIf<IsEnumerable<Enumerable_>::Value, void>::ValueT>
		{
			using ValueT = EnumerableToRange<Enumerable_>;

			static ValueT Do(const Enumerable_& range)
			{ return ValueT(range); }
		};
	}

	/** @} */

}

#endif
