#ifndef STINGRAY_TOOLKIT_IBYTESTREAM_H
#define STINGRAY_TOOLKIT_IBYTESTREAM_H


#include <stingray/toolkit/SeekMode.h>
#include <stingray/toolkit/shared_ptr.h>


namespace stingray
{

	struct IByteStream
	{
		virtual ~IByteStream() { }

		virtual size_t Read(void* data, size_t count) = 0;
		virtual size_t Write(const void* data, size_t count) = 0;
		virtual void Seek(int offset, SeekMode mode = SeekMode::Begin) = 0;
		virtual size_t Tell() const = 0;
	};
	TOOLKIT_DECLARE_PTR(IByteStream);

}


#endif
