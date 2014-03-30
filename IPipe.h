#ifndef STINGRAY_TOOLKIT_IPIPE_H
#define STINGRAY_TOOLKIT_IPIPE_H


#include <stingray/threads/CancellationToken.h>
#include <stingray/toolkit/shared_ptr.h>


namespace stingray
{

	struct IPipe
	{
		virtual ~IPipe() {}

		virtual size_t Read(ByteData dst, const CancellationToken& token) const = 0;
		virtual size_t Write(ConstByteData src, const CancellationToken& token) const = 0;
	};
	TOOLKIT_DECLARE_PTR(IPipe);

}


#endif
