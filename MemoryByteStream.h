#ifndef STINGRAY_TOOLKIT_MEMORYBYTESTREAM_H
#define STINGRAY_TOOLKIT_MEMORYBYTESTREAM_H


#include <stingray/toolkit/IByteStream.h>
#include <stingray/toolkit/ByteData.h>


namespace stingray
{


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

		virtual u64 Read(void* data, u64 count)
		{
			u8* dst = static_cast<u8*>(data);
			count = std::min(count, _data.size() - _offset);
			std::copy(_data.begin() + _offset, _data.begin() + _offset + count, dst);
			_offset += count;
			return count;
		}

		virtual u64 Write(const void* data, u64 count)
		{ TOOLKIT_THROW(NotImplementedException()); }

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
				TOOLKIT_THROW(IndexOutOfRangeException());
			if (new_ofs > (int)_data.size())
				TOOLKIT_THROW(NotImplementedException());
			_offset = new_ofs;
		}

		virtual u64 Tell() const
		{ return _offset; }
	};


	template < typename ContainerType >
	IByteStreamPtr CreateMemoryByteStream(const ContainerType& data)
	{ return make_shared<MemoryByteStream<ContainerType> >(data); }

}


#endif
