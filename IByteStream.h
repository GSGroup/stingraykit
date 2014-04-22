#ifndef STINGRAY_TOOLKIT_IBYTESTREAM_H
#define STINGRAY_TOOLKIT_IBYTESTREAM_H


#include <stingray/toolkit/SeekMode.h>
#include <stingray/toolkit/shared_ptr.h>
#include <stingray/toolkit/ICreator.h>
#include <stingray/toolkit/ByteData.h>


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
	TOOLKIT_DECLARE_PTR(IByteStream);
	TOOLKIT_DECLARE_CREATOR(IByteStream);

}


#endif
