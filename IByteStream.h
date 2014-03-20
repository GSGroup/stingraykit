#ifndef STINGRAY_TOOLKIT_IBYTESTREAM_H
#define STINGRAY_TOOLKIT_IBYTESTREAM_H


#include <stingray/toolkit/SeekMode.h>
#include <stingray/toolkit/shared_ptr.h>
#include <stingray/toolkit/ICreator.h>


namespace stingray
{

	struct IByteStream
	{
		virtual ~IByteStream() { }

		virtual u64 Read(void* data, u64 count) = 0;
		virtual u64 Write(const void* data, u64 count) = 0;

		template<typename Data>
		inline u64 ReadData(Data data)
		{ return Read(data.data(), data.size()); }

		template<typename Data>
		inline u64 WriteData(Data data)
		{ return Write(data.data(), data.size()); }

		virtual void Seek(s64 offset, SeekMode mode = SeekMode::Begin) = 0;
		virtual u64 Tell() const = 0;
	};
	TOOLKIT_DECLARE_PTR(IByteStream);
	TOOLKIT_DECLARE_CREATOR(IByteStream);

}


#endif
