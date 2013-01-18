#ifndef STINGRAY_TOOLKIT_ZIPSTREAM_H
#define STINGRAY_TOOLKIT_ZIPSTREAM_H

#include <stingray/toolkit/ByteData.h>

namespace stingray
{
	class ZipStream
	{
	public:
		static bool CheckGzipHeader(const ConstByteData &src);
		static ByteArray Compress(const ConstByteData &src, int level, bool gzHeader);
		static ByteArray Decompress(const ConstByteData &src, bool gzHeader);
	};
}

#endif
