#ifndef STINGRAYKIT_IO_IBYTESTREAM_H
#define STINGRAYKIT_IO_IBYTESTREAM_H


#include <stingray/toolkit/io/SeekMode.h>
#include <stingray/toolkit/shared_ptr.h>
#include <stingray/toolkit/ICreator.h>
#include <stingray/toolkit/collection/ByteData.h>


namespace stingray
{

	struct IByteStream
	{
		virtual ~IByteStream() { }

		virtual u64 Read(ByteData data) = 0;
		virtual u64 Write(ConstByteData data) = 0;

		virtual void Seek(s64 offset, SeekMode mode = SeekMode::Begin) = 0;
		virtual u64 Tell() const = 0;
	};
	STINGRAYKIT_DECLARE_PTR(IByteStream);
	STINGRAYKIT_DECLARE_CREATOR(IByteStream);


	class ByteStreamWithOffset : public IByteStream
	{
		IByteStreamPtr	_stream;
		const size_t	_offset;

	public:
		ByteStreamWithOffset(const IByteStreamPtr & stream, size_t offset) : _stream(stream), _offset(offset)
		{ _stream->Seek(offset); }

		virtual u64 Tell() const
		{
			size_t tell = _stream->Tell();
			STINGRAYKIT_CHECK(tell >= _offset, "Wrong ByteStreamWithOffset state");
			return tell - _offset;
		}

		virtual void Seek(s64 offset, SeekMode mode = SeekMode::Begin)
		{ _stream->Seek(mode == SeekMode::Begin ? static_cast<s64>(_offset) + offset : offset, mode); }

		virtual u64 Read(ByteData data)									{ return _stream->Read(data); }
		virtual u64 Write(ConstByteData data)							{ return _stream->Write(data); }
	};

}


#endif
