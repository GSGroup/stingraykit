// Copyright (c) 2011 - 2024, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#ifndef STINGRAYKIT_COMPRESSION_PPM_PPMDICTIONARYFACTORY_H
#define STINGRAYKIT_COMPRESSION_PPM_PPMDICTIONARYFACTORY_H

#include <stingraykit/compression/ICompressionDictionary.h>
#include <stingraykit/compression/ppm/PpmDictionaryBuilder.h>

namespace stingray
{

	template <typename Symbol_, typename SymbolCount_>
	struct PpmConfig
	{
		typedef Symbol_      Symbol;
		typedef SymbolCount_ SymbolCount;
	};


	template <size_t ContextSize_, typename Probability_>
	struct PpmModelConfig
	{
		static const size_t ContextSize = ContextSize_;
		typedef Probability_ Probability;
		static const size_t Scale = IntTraits<Probability>::Max;
	};


	template <typename PpmConfig_, typename ModelConfigList_>
	class PpmDictionaryFactory : public virtual ICompressionDictionaryFactory
	{
		typedef PpmDictionaryBuilder<PpmConfig_, ModelConfigList_>  DictionaryBuilder;

	private:
		const size_t _lowMemoryThreshold;
		const size_t _highMemoryThreshold;
		const size_t _finalMemoryThreshold;

	public:
		PpmDictionaryFactory(size_t lowMemoryThreshold, size_t highMemoryThreshold, size_t finalMemoryThreshold) :
			_lowMemoryThreshold(lowMemoryThreshold), _highMemoryThreshold(highMemoryThreshold),
			_finalMemoryThreshold(finalMemoryThreshold)
		{ }

		virtual ICompressionDictionaryBuilderPtr CreateBuilder() const
		{ return make_shared_ptr<DictionaryBuilder>(_lowMemoryThreshold, _highMemoryThreshold, _finalMemoryThreshold); }
	};

}

#endif
