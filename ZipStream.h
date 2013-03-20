#ifndef STINGRAY_TOOLKIT_ZIPSTREAM_H
#define STINGRAY_TOOLKIT_ZIPSTREAM_H

#include <stingray/toolkit/ByteData.h>
#include <stingray/log/Logger.h>

namespace stingray
{
	class ZipStream
	{
		static NamedLogger s_logger;
	public:
		static bool CheckGzipHeader(const ConstByteData &src);
		static ByteArray Compress(const ConstByteData &src, int level, bool gzHeader);
		static ByteArray Decompress(const ConstByteData &src, bool gzHeader);
	};
}

#endif
