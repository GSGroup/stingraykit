#ifndef STINGRAYKIT_COLLECTION_BYTESOWNER_H
#define STINGRAYKIT_COLLECTION_BYTESOWNER_H

// Copyright (c) 2011 - 2022, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/collection/ByteData.h>
#include <stingraykit/Token.h>

namespace stingray
{

	template < typename T >
	class BasicBytesOwner
	{
		template < typename >
		friend class BasicBytesOwner;

	private:
		using DataType = BasicByteData<T>;

	public:
		using value_type = T;

		using iterator = typename DataType::iterator;
		using const_iterator = typename DataType::const_iterator;

		using reverse_iterator = typename DataType::reverse_iterator;
		using const_reverse_iterator = typename DataType::const_reverse_iterator;

	private:
		using DeconstT = typename RemoveConst<T>::ValueT;

		class Storage final : public virtual IToken
		{
		private:
			unique_ptr<T[]>		_data;

		public:
			Storage(unique_ptr<T[]>&& data)
				:	_data(std::move(data))
			{ }
		};

	private:
		DataType				_data;
		Token					_lifeAssurance;

	public:
		BasicBytesOwner()
		{ }

		BasicBytesOwner(DataType data, const Token& lifeAssurance)
			:	_data(data),
				_lifeAssurance(lifeAssurance)
		{ }

		template < typename U, typename EnableIf<IsConvertible<U*, T*>::Value, bool>::ValueT = false >
		BasicBytesOwner(const BasicBytesOwner<U>& other)
			:	_data(other._data),
				_lifeAssurance(other._lifeAssurance)
		{ }

		template < typename U, typename EnableIf<IsConvertible<U*, T*>::Value, bool>::ValueT = false >
		BasicBytesOwner(const BasicBytesOwner<U>& other, size_t offset)
			:	_data(other._data, offset),
				_lifeAssurance(other._lifeAssurance)
		{ }

		template < typename U, typename EnableIf<IsConvertible<U*, T*>::Value, bool>::ValueT = false >
		BasicBytesOwner(const BasicBytesOwner<U>& other, size_t offset, size_t size)
			:	_data(other._data, offset, size),
				_lifeAssurance(other._lifeAssurance)
		{ }

		T& operator [] (size_t index) const				{ return _data[index]; }

		T* data() const									{ return _data.data(); }
		size_t size() const								{ return _data.size(); }

		bool empty() const								{ return _data.size() == 0; }

		iterator begin() const							{ return _data.begin(); }
		iterator end() const							{ return _data.end(); }

		reverse_iterator rbegin() const					{ return _data.rbegin(); }
		reverse_iterator rend() const					{ return _data.rend(); }

		operator BasicByteData<DeconstT> () const		{ return _data; }
		operator BasicByteData<const DeconstT> () const	{ return _data; }

		bool operator == (const BasicBytesOwner& other) const
		{ return _data == other._data; }

		STINGRAYKIT_GENERATE_EQUALITY_OPERATORS_FROM_EQUAL(BasicBytesOwner);

		bool operator < (const BasicBytesOwner& other) const
		{ return _data < other._data; }

		STINGRAYKIT_GENERATE_RELATIONAL_OPERATORS_FROM_LESS(BasicBytesOwner);

		static BasicBytesOwner Create(size_t size)
		{
			unique_ptr<T[]> arr(make_unique_ptr<T[]>(size));

			const DataType data(arr.get(), size);
			return BasicBytesOwner(data, MakeToken<Storage>(std::move(arr)));
		}

		static BasicBytesOwner Create(BasicByteData<const T> other)
		{
			unique_ptr<DeconstT[]> arr(make_unique_ptr<DeconstT[]>(other.size()));
			std::copy(other.begin(), other.end(), arr.get());

			const DataType data(arr.get(), other.size());
			return BasicBytesOwner(data, MakeToken<Storage>(std::move(arr)));
		}
	};


	using ConstBytesOwner = BasicBytesOwner<const u8>;
	using BytesOwner = BasicBytesOwner<u8>;

}

#endif
