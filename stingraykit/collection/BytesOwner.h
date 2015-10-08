#ifndef STINGRAYKIT_COLLECTION_BYTESOWNER_H
#define STINGRAYKIT_COLLECTION_BYTESOWNER_H

// Copyright (c) 2011 - 2015, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/Token.h>
#include <stingraykit/collection/ByteData.h>
#include <stingraykit/unique_ptr.h>

namespace stingray
{

	template < typename T >
	class BasicBytesOwner
	{
		struct Storage : public IToken
		{
		private:
			T* _data;

		public:
			Storage(T* data) : _data(data)
			{ }

			virtual ~Storage()
			{ delete[] _data; }
		};

		typedef BasicByteData<T>							DataType;
		typedef typename Deconst<T>::ValueT					DeconstT;

	public:
		typedef T											value_type;

		typedef typename DataType::iterator					iterator;
		typedef typename DataType::const_iterator			const_iterator;

		typedef typename DataType::reverse_iterator			reverse_iterator;
		typedef typename DataType::const_reverse_iterator	const_reverse_iterator;

	private:
		DataType	_data;
		Token		_lifeAssurance;

	public:
		BasicBytesOwner()
		{ }

		BasicBytesOwner(DataType data, const Token& lifeAssurance) :
			_data(data), _lifeAssurance(lifeAssurance)
		{ }

		BasicBytesOwner(const BasicBytesOwner& other, size_t offset) :
			_data(other._data, offset), _lifeAssurance(other._lifeAssurance)
		{ }

		BasicBytesOwner(const BasicBytesOwner& other, size_t offset, size_t size) :
			_data(other._data, offset, size), _lifeAssurance(other._lifeAssurance)
		{ }

		T& operator[](size_t index) const				{ return _data[index]; }

		T* data() const									{ return _data.data(); }
		size_t size() const								{ return _data.size(); }

		bool empty() const								{ return _data.size() == 0; }

		iterator begin() const							{ return _data.begin(); }
		iterator end() const							{ return _data.end(); }

		reverse_iterator rbegin() const					{ return _data.rbegin(); }
		reverse_iterator rend() const					{ return _data.rend(); }

		operator BasicByteData<DeconstT>() const		{ return _data; }
		operator BasicByteData<const DeconstT>() const	{ return _data; }

		bool operator == (const BasicBytesOwner& other) const
		{ return _data == other._data; }

		STINGRAYKIT_GENERATE_EQUALITY_OPERATORS_FROM_EQUAL(BasicBytesOwner);

		bool operator < (const BasicBytesOwner& other) const
		{ return _data < other._data; }

		STINGRAYKIT_GENERATE_RELATIONAL_OPERATORS_FROM_LESS(BasicBytesOwner);

		static BasicBytesOwner Create(size_t size)
		{
			unique_ptr<DeconstT[]> arr(new T[size]);
			BasicBytesOwner result(DataType(arr.get(), size), MakeToken<Storage>(arr.get()));
			arr.release();
			return result;
		}

		static BasicBytesOwner Create(BasicByteData<const T> data)
		{
			unique_ptr<DeconstT[]> arr(new DeconstT[data.size()]);
			std::copy(data.begin(), data.end(), arr.get());
			BasicBytesOwner result(DataType(arr.get(), data.size()), MakeToken<Storage>(arr.get()));
			arr.release();
			return result;
		}
	};

	typedef BasicBytesOwner<const u8>	ConstBytesOwner;
	typedef BasicBytesOwner<u8>			BytesOwner;

}

#endif
