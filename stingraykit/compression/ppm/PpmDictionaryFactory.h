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


	template <size_t ContextSize_, size_t ProbabilityScale_>
	struct PpmModelConfig
	{
		static const size_t ContextSize = ContextSize_;
		static const size_t Scale = ProbabilityScale_;
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
		{ return make_shared<DictionaryBuilder>(_lowMemoryThreshold, _highMemoryThreshold, _finalMemoryThreshold); }
	};

}

#endif
