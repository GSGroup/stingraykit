// Copyright (c) 2011 - 2015, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/io/BithreadCircularBuffer.h>

namespace stingray
{

	struct BithreadCircularBuffer::Impl : public self_counter<BithreadCircularBuffer::Impl>
	{
		// TODO: finish padding support
		static const size_t PaddingSize = 8;

	private:
		BytesOwner		_storage;
		size_t			_writeOffset, _readOffset;
		bool			_dataIsContiguous;

	public:
		Impl(const BytesOwner& storage) :
			_storage(storage),
			_writeOffset(0), _readOffset(0),
			_dataIsContiguous(true)
		{ }


		size_t GetDataSize() const
		{ return _dataIsContiguous ? (_writeOffset - _readOffset) : (GetStorageSize() - _readOffset + _writeOffset); }


		size_t GetFreeSize() const
		{ return GetStorageSize() - GetDataSize() - 1; }


		size_t GetStorageSize() const
		{ return _storage.size(); }


		ConstByteData GetStorage() const
		{ return _storage; }


		ByteData Write()
		{
			size_t size = _dataIsContiguous ? (GetStorageSize() - _writeOffset) : (_readOffset - _writeOffset);
			return ByteData(_storage, _writeOffset, size);
		}


		void Push(size_t pushSize)
		{
			STINGRAYKIT_CHECK(_writeOffset + pushSize <= GetStorageSize(), IndexOutOfRangeException(pushSize, _writeOffset, GetStorageSize()));
			_writeOffset += pushSize;

			if (_writeOffset == GetStorageSize())
			{
				_writeOffset = 0;
				_dataIsContiguous = !_dataIsContiguous;
			}
		}


		ConstByteData Read()
		{
			size_t size = (_dataIsContiguous) ? (_writeOffset - _readOffset) : (GetStorageSize() - _readOffset);
			return ByteData(_storage, _readOffset, size);
		}


		void Pop(size_t popSize)
		{
			STINGRAYKIT_CHECK(_readOffset + popSize <= GetStorageSize(), IndexOutOfRangeException(popSize, _readOffset, GetStorageSize()));
			_readOffset += popSize;

			if (_readOffset == GetStorageSize())
			{
				_readOffset = 0;
				_dataIsContiguous = !_dataIsContiguous;
			}
		}


		void Clear()
		{
			_writeOffset = 0;
			_readOffset = 0;
			_dataIsContiguous = true;
		}
	};


	BithreadCircularBuffer::BithreadCircularBuffer(size_t size) : _impl(new Impl(BytesOwner::Create(size)))
	{ }


	BithreadCircularBuffer::BithreadCircularBuffer(const BytesOwner& storage) : _impl(new Impl(storage))
	{ }


	BithreadCircularBuffer::~BithreadCircularBuffer()
	{ }


	size_t BithreadCircularBuffer::GetDataSize() const
	{ return _impl->GetDataSize(); }


	size_t BithreadCircularBuffer::GetFreeSize() const
	{ return _impl->GetFreeSize(); }


	size_t BithreadCircularBuffer::GetTotalSize() const
	{ return _impl->GetStorageSize(); }


	BithreadCircularBuffer::Reader BithreadCircularBuffer::Read()
	{ return Reader(_impl); }


	BithreadCircularBuffer::Writer BithreadCircularBuffer::Write()
	{ return Writer(_impl); }


	void BithreadCircularBuffer::Clear()
	{ _impl->Clear(); }


	BithreadCircularBuffer::Reader::Reader(const ImplSelfCountPtr& impl) :
		_impl(impl),
		_data(impl->Read())
	{ }


	BithreadCircularBuffer::Reader::Reader(const Reader& other) : _impl(other._impl), _data(other._data)
	{ }


	BithreadCircularBuffer::Reader::~Reader()
	{ }


	BithreadCircularBuffer::Reader& BithreadCircularBuffer::Reader::operator =(const Reader& other)
	{
		_impl = other._impl;
		_data = other._data;
		return *this;
	}


	const u8* BithreadCircularBuffer::Reader::data() const
	{ return _data.data(); }


	size_t BithreadCircularBuffer::Reader::size() const
	{ return _data.size(); }


	ConstByteData::iterator BithreadCircularBuffer::Reader::begin()
	{ return _data.begin(); }


	ConstByteData::iterator BithreadCircularBuffer::Reader::end()
	{ return _data.end(); }


	ConstByteData BithreadCircularBuffer::Reader::GetData() const
	{ return _data; }


	bool BithreadCircularBuffer::Reader::IsBufferEnd() const
	{
		ConstByteData storage(_impl->GetStorage());
		return (_data.data() + _data.size()) == (storage.data() + storage.size());
	}


	void BithreadCircularBuffer::Reader::Pop(size_t bytes)
	{
		_impl->Pop(bytes);
		_data = ConstByteData(NULL, 0);
	}


	BithreadCircularBuffer::Writer::Writer(const ImplSelfCountPtr& impl) :
		_impl(impl),
		_data(impl->Write())
	{ }


	BithreadCircularBuffer::Writer::Writer(const Writer& other) : _impl(other._impl), _data(other._data)
	{ }


	BithreadCircularBuffer::Writer::~Writer()
	{ }


	BithreadCircularBuffer::Writer& BithreadCircularBuffer::Writer::operator =(const Writer& other)
	{
		_impl = other._impl;
		_data = other._data;
		return *this;
	}


	u8* BithreadCircularBuffer::Writer::data() const
	{ return _data.data(); }


	size_t BithreadCircularBuffer::Writer::size() const
	{ return _data.size(); }


	ByteData::iterator BithreadCircularBuffer::Writer::begin()
	{ return _data.begin(); }


	ByteData::iterator BithreadCircularBuffer::Writer::end()
	{ return _data.end(); }


	ByteData BithreadCircularBuffer::Writer::GetData() const
	{ return _data; }


	bool BithreadCircularBuffer::Writer::IsBufferEnd() const
	{
		ConstByteData storage(_impl->GetStorage());
		return (_data.data() + _data.size()) == (storage.data() + storage.size());
	}


	void BithreadCircularBuffer::Writer::Push(size_t bytes)
	{
		_impl->Push(bytes);
		_data = ByteData(null, 0);
	}

}
