#ifndef STINGRAYKIT_IO_MEMORYBYTESTREAM_H
#define STINGRAYKIT_IO_MEMORYBYTESTREAM_H

// Copyright (c) 2011 - 2018, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.


#include <stingraykit/collection/ByteData.h>
#include <stingraykit/io/IByteStream.h>


namespace stingray
{

	namespace Detail
	{
		template < typename ContainerType >
		struct MemoryByteStreamContainerResizer
		{
			static void RequireSize(ContainerType& container, u64 size)
			{ }
		};

		template < >
		struct MemoryByteStreamContainerResizer<ByteArray>
		{
			static void RequireSize(ByteArray& container, u64 size)
			{ container.RequireSize(size); }
		};


		template < typename ContainerType, bool IsConstContainer = IsConst<typename ContainerType::value_type>::Value >
		struct MemoryByteStreamWriter
		{
			static u64 Write(ContainerType& container, u64& offset, ConstByteData data)
			{
				if (container.size() - offset < data.size())
					Detail::MemoryByteStreamContainerResizer<ContainerType>::RequireSize(container, offset + data.size());
				size_t count = std::min<size_t>(data.size(), container.size() - offset);
				std::copy(data.data(), data.data() + count, container.begin() + offset);
				offset += count;
				return count;
			}
		};

		template < typename ContainerType >
		struct MemoryByteStreamWriter<ContainerType, true>
		{
			static u64 Write(ContainerType& container, u64& offset, ConstByteData data)
			{ STINGRAYKIT_THROW("Cannot write data to a const container!"); }
		};
	}


	template < typename ContainerType >
	class MemoryByteStream : public virtual IByteStream
	{
	private:
		ContainerType	_data;
		u64				_offset;

	public:
		MemoryByteStream(const ContainerType& data)
			: _data(data), _offset(0)
		{ }

		ContainerType GetData() const { return _data; }

		virtual u64 Read(ByteData data, const ICancellationToken& token)
		{
			size_t count = std::min<size_t>(data.size(), _data.size() - _offset);
			std::copy(_data.begin() + _offset, _data.begin() + _offset + count, data.data());
			_offset += count;
			return count;
		}

		virtual u64 Write(ConstByteData data, const ICancellationToken& token)
		{ return Detail::MemoryByteStreamWriter<ContainerType>::Write(_data, _offset, data); }

		virtual void Seek(s64 offset, SeekMode mode = SeekMode::Begin)
		{
			s64 new_ofs = _offset;
			switch (mode)
			{
			case SeekMode::Begin:	new_ofs = offset; break;
			case SeekMode::Current:	new_ofs += offset; break;
			case SeekMode::End:		new_ofs = _data.size() + offset; break;
			}
			if (new_ofs < 0)
				STINGRAYKIT_THROW(IndexOutOfRangeException());
			if (new_ofs > (int)_data.size())
				STINGRAYKIT_THROW(NotImplementedException());
			_offset = new_ofs;
		}

		virtual u64 Tell() const
		{ return _offset; }
	};


	template < typename ContainerType >
	IByteStreamPtr CreateMemoryByteStream(const ContainerType& data)
	{ return make_shared<MemoryByteStream<ContainerType> >(data); }

	inline IByteStreamPtr CreateMemoryByteStream(ConstByteData data)
	{ return  make_shared<MemoryByteStream<ConstByteData> >(data); }

	typedef MemoryByteStream<ConstByteArray>	ConstByteArrayByteStream;
	STINGRAYKIT_DECLARE_PTR(ConstByteArrayByteStream);
	typedef MemoryByteStream<ByteArray>			ByteArrayByteStream;
	STINGRAYKIT_DECLARE_PTR(ByteArrayByteStream);
	typedef MemoryByteStream<ConstByteData>		ConstByteDataByteStream;
	STINGRAYKIT_DECLARE_PTR(ConstByteDataByteStream);
	typedef MemoryByteStream<ByteData>			ByteDataByteStream;
	STINGRAYKIT_DECLARE_PTR(ByteDataByteStream);

}


#endif
