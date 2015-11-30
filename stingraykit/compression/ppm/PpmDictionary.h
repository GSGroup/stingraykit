#ifndef STINGRAYKIT_COMPRESSION_PPM_PPMDICTIONARY_H
#define STINGRAYKIT_COMPRESSION_PPM_PPMDICTIONARY_H

#include <stingraykit/compression/ICompressionDictionary.h>
#include <stingraykit/compression/ppm/PpmCompressor.h>
#include <stingraykit/compression/ppm/PpmDecompressor.h>
#include <stingraykit/compression/ppm/PpmModel.h>

namespace stingray
{

	template <typename PpmConfig_, typename ModelConfigList_>
	class PpmDictionary : public virtual ICompressionDictionary
	{
	public:
		typedef PpmImpl<PpmConfig_>                             Impl;
		typedef typename Impl::template Model<ModelConfigList_> Model;
		typedef PpmCompressor<PpmConfig_, ModelConfigList_>     Compressor;
		typedef PpmDecompressor<PpmConfig_, ModelConfigList_>   Decompressor;

	private:
		shared_ptr<const Model>    _model;

	public:
		PpmDictionary(const shared_ptr<const Model>& model) : _model(model)
		{ }

		virtual ICompressorPtr Compress(const IDataSourcePtr& source) const
		{ return make_shared<Compressor>(_model, source); }

		virtual IDecompressorPtr Decompress(const IDataSourcePtr& source) const
		{ return make_shared<Decompressor>(_model, source); }

		virtual std::string ToString() const
		{ return StringBuilder() % _model; }
	};

}

#endif
