#ifndef STINGRAYKIT_IO_IOUTPUTBYTESTREAM_H
#define STINGRAYKIT_IO_IOUTPUTBYTESTREAM_H

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
