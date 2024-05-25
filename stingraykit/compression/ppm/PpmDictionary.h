#ifndef STINGRAYKIT_COMPRESSION_PPM_PPMDICTIONARY_H
#define STINGRAYKIT_COMPRESSION_PPM_PPMDICTIONARY_H

// Copyright (c) 2011 - 2024, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

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
		{ return make_shared_ptr<Compressor>(_model, source); }

		virtual IDecompressorPtr Decompress(const IDataSourcePtr& source) const
		{ return make_shared_ptr<Decompressor>(_model, source); }

		virtual std::string ToString() const
		{ return StringBuilder() % _model; }
	};

}

#endif
