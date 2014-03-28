#ifndef STINGRAY_TOOLKIT_IPIPE_H
#define STINGRAY_TOOLKIT_IPIPE_H


#include <stingray/threads/CancellationToken.h>
#include <stingray/toolkit/shared_ptr.h>


namespace stingray
{

	struct IPipe
	{
		virtual ~IPipe() {}

		ConstByteArray Read(size_t size, const CancellationToken& token) const = 0;
		void Write(ConstByteData data, const CancellationToken& token) const = 0;
	};
	TOOLKIT_DECLARE_PTR(IPipe);

}


#endif
