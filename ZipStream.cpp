#include <stingray/toolkit/ZipStream.h>
#include <stingray/toolkit/ScopeExit.h>
#include <stingray/toolkit/bind.h>
#include <stingray/toolkit/exception.h>
#include <stingray/toolkit/StringUtils.h>

#include <zlib.h>
#include <string.h>


namespace stingray
{

	namespace
	{
		static const size_t BufferSize = 128 * 1024;

		struct ZlibException : public Exception
		{
			ZlibException(const z_stream& z, const std::string &method, int ret): Exception(StringBuilder() % "zlib " % method % " failed: " % z.msg % " with code = " % ret ) {}
		};
	}

	ByteArray Compress(const ConstByteData &src, int level, bool gzHeader)
	{
		z_stream z = {};
		ScopeExitInvoker sei(bind(&deflateEnd, &z));

		int ret;
		z.avail_in = src.size();
		z.next_in = (Bytef *) src.data();

		if ((ret = deflateInit2(&z, level, Z_DEFLATED, gzHeader? 0x1f: 0x0f, 8, Z_DEFAULT_STRATEGY)) != Z_OK)
			TOOLKIT_THROW(ZlibException(z, "DeflateInit", ret));

		std::vector<u8> dst(BufferSize);

		while(true)
		{
			z.avail_out = dst.size() - z.total_out;
			z.next_out = (Bytef*)&dst[z.total_out];

			ret = deflate(&z, Z_FINISH);

			if (ret == Z_STREAM_END)
				break;

			if (ret == Z_BUF_ERROR || z.avail_out == 0)
			{
				if (z.avail_out == 0)
				{
					dst.resize(dst.size() + BufferSize);
					continue;
				}
				else if (z.avail_in == 0)
					TOOLKIT_THROW("deflate: stream was truncated. unable to proceed.");
			}

			if (ret != Z_OK)
				TOOLKIT_THROW(ZlibException(z, "deflate", ret));
		}

		if ((ret = deflateEnd(&z)) != Z_OK)
			TOOLKIT_THROW(ZlibException(z, "deflateEnd", ret));

		dst.resize(z.total_out);
		return ByteArray(dst.begin(), dst.end());
	}

	ByteArray Decompress(const ConstByteData &src, bool gzHeader)
	{
		z_stream z = {};
		ScopeExitInvoker sei(bind(&inflateEnd, &z));

		int ret;
		z.avail_in = src.size();
		z.next_in = (Bytef*) src.data();

		if ((ret = inflateInit2(&z, gzHeader? 0x1f: 0x0f)) != Z_OK)
			TOOLKIT_THROW(ZlibException(z, "inflateInit", ret));

		std::vector<u8> dst;
		dst.resize(BufferSize);

		while(z.avail_in != 0)
		{
			z.avail_out = dst.size() - z.total_out;
			z.next_out = (Bytef*)&dst[z.total_out];

			ret = inflate(&z, Z_FINISH);

			if (ret == Z_STREAM_END)
				break;

			if (ret == Z_BUF_ERROR || z.avail_out == 0)
			{
				if (z.avail_out == 0)
				{
					dst.resize(dst.size() + BufferSize);
					continue;
				}
				else if (z.avail_in == 0)
					TOOLKIT_THROW("stream was truncated. unable to proceed.");
			}

			if (ret != Z_OK)
				TOOLKIT_THROW(ZlibException(z, "inflate", ret));
		}

		if ((ret = inflateEnd(&z)) != Z_OK)
			TOOLKIT_THROW(ZlibException(z, "inflateEnd", ret));

		dst.resize(z.total_out);
		return ByteArray(dst.begin(), dst.end());
	}

}
