#ifndef STINGRAYKIT_COMPRESSION_ICOMPRESSIONENGINE_H
#define STINGRAYKIT_COMPRESSION_ICOMPRESSIONENGINE_H

#include <stingraykit/io/IDataSource.h>
#include <stingraykit/string/IStringRepresentable.h>

namespace stingray
{

	struct IDecompressor : public virtual IDataSource
	{ };
	STINGRAYKIT_DECLARE_PTR(IDecompressor);


	struct ICompressor : public virtual IDataSource
	{ };
	STINGRAYKIT_DECLARE_PTR(ICompressor);


	struct ICompressionEngine : public virtual IStringRepresentable
	{
		virtual ~ICompressionEngine() { }

		virtual IDecompressorPtr Decompress(const IDataSourcePtr& source) const = 0;
		virtual ICompressorPtr Compress(const IDataSourcePtr& source) const = 0;
	};
	STINGRAYKIT_DECLARE_PTR(ICompressionEngine);

}

#endif
