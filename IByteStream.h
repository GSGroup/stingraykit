#ifndef __GS_STINGRAY_TOOLKIT_IBYTESTREAM_H__
#define __GS_STINGRAY_TOOLKIT_IBYTESTREAM_H__


#include <stingray/toolkit/shared_ptr.h>


namespace stingray
{

	TOOLKIT_DECLARE_SIMPLE_EXCEPTION(EndOfStreamException, "End of stream reached!");


	struct SeekOrigin
	{
		TOOLKIT_ENUM_VALUES
		(
			Begin,
			Current,
			End
		);
		TOOLKIT_DECLARE_ENUM_CLASS(SeekOrigin);
	};


	struct IByteStream
	{
		virtual ~IByteStream() { }

		virtual size_t Read(void* data, size_t count) = 0;
		virtual size_t Write(void* data, size_t count) = 0;
		virtual void Seek(ssize_t offset, SeekOrigin origin) = 0;

		virtual bool CanRead() const = 0;
		virtual bool CanWrite() const = 0;
		virtual bool CanSeek() const = 0;

		virtual size_t GetPosition() const = 0;
		virtual size_t GetLength() const = 0;
	};
	TOOLKIT_DECLARE_PTR(IByteStream);

}


#endif
