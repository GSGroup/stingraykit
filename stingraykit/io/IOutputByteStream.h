#ifndef STINGRAY_STINGRAYKIT_STINGRAYKIT_IO_IOUTPUTBYTESTREAM_H
#define STINGRAY_STINGRAYKIT_STINGRAYKIT_IO_IOUTPUTBYTESTREAM_H

#include <stingraykit/collection/ByteData.h>
#include <stingraykit/thread/DummyCancellationToken.h>

namespace stingray
{

	struct IOutputByteStream
	{
		virtual ~IOutputByteStream() { }

		virtual u64 Write(ConstByteData data, const ICancellationToken& token = DummyCancellationToken()) = 0;
	};
	STINGRAYKIT_DECLARE_PTR(IOutputByteStream);

}

#endif
