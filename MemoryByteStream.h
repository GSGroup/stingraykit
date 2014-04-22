#ifndef STINGRAY_TOOLKIT_MEMORYBYTESTREAM_H
#define STINGRAY_TOOLKIT_MEMORYBYTESTREAM_H


#include <stingray/toolkit/ByteData.h>
#include <stingray/toolkit/IByteStream.h>
#include <stingray/toolkit/MetaProgramming.h>


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
			{ TOOLKIT_THROW("Cannot write data to a const container!"); }
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

		virtual u64 Read(ByteData data)
		{
			size_t count = std::min<size_t>(data.size(), _data.size() - _offset);
			std::copy(_data.begin() + _offset, _data.begin() + _offset + count, data.data());
			_offset += count;
			return count;
		}

		virtual u64 Write(ConstByteData data)
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

	IByteStreamPtr CreateMemoryByteStream(ConstByteData data)
	{ return  make_shared<MemoryByteStream<ConstByteData> >(data); }

}


#endif
