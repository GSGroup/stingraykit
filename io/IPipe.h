#ifndef STINGRAYKIT_IO_IPIPE_H
#define STINGRAYKIT_IO_IPIPE_H


#include <stingray/toolkit/thread/CancellationToken.h>
#include <stingray/toolkit/collection/ByteData.h>
#include <stingray/toolkit/shared_ptr.h>


namespace stingray
{

	struct IPipe
	{
		virtual ~IPipe() {}

		virtual size_t Read(ByteData dst, const ICancellationToken& token) = 0;
		virtual size_t Write(ConstByteData src, const ICancellationToken& token) = 0;
	};
	STINGRAYKIT_DECLARE_PTR(IPipe);

}


#endif
