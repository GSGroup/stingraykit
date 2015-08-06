#ifndef STINGRAYKIT_IO_IINPUTBYTESTREAM_H
#define STINGRAYKIT_IO_IINPUTBYTESTREAM_H

#include <stingraykit/collection/ByteData.h>
#include <stingraykit/thread/DummyCancellationToken.h>

namespace stingray
{

	struct IInputByteStream
	{
		virtual ~IInputByteStream() { }

		virtual u64 Read(ByteData data, const ICancellationToken& token = DummyCancellationToken()) = 0;
	};
	STINGRAYKIT_DECLARE_PTR(IInputByteStream);

}

#endif
