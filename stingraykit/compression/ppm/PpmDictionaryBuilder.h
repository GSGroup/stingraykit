#ifndef STINGRAYKIT_COMPRESSION_PPM_PPMDICTIONARYBUILDER_H
#define STINGRAYKIT_COMPRESSION_PPM_PPMDICTIONARYBUILDER_H


#include <stingraykit/compression/ICompressionDictionary.h>
#include <stingraykit/compression/ppm/PpmDictionary.h>
#include <stingraykit/compression/ppm/PpmModel.h>

#include <deque>

namespace stingray
{

	template <typename PpmConfig_, typename ModelConfigList_>
	class PpmDictionaryBuilder : public virtual ICompressionDictionaryBuilder
	{
	public:
		typedef typename PpmConfig_::Symbol                 Symbol;
		typedef typename PpmConfig_::SymbolCount            SymbolCount;
		typedef PpmModel<PpmConfig_, ModelConfigList_>      Model;
		typedef PpmDictionary<PpmConfig_, ModelConfigList_> Dictionary;

	private:
		const size_t         _lowMemoryThreshold;
		const size_t         _highMemoryThreshold;
		const size_t         _finalMemoryThreshold;
		size_t               _memoryConsumption;
		shared_ptr<Model>    _model;
		std::deque<Symbol>   _context;

	public:
		PpmDictionaryBuilder(size_t lowMemoryThreshold, size_t highMemoryThreshold, size_t finalMemoryThreshold) :
			_lowMemoryThreshold(lowMemoryThreshold), _highMemoryThreshold(highMemoryThreshold),
			_finalMemoryThreshold(finalMemoryThreshold), _memoryConsumption(0),
			_model(make_shared<Model>())
		{ }

		virtual void Process(ConstByteData data)
		{
			for (ConstByteData::const_iterator it = data.begin(); it != data.end(); ++it)
				AddSymbol(*it);
		}

		virtual ICompressionDictionaryPtr Finish()
		{
			typename Model::Compactificator compactificator(*_model);
			while (_memoryConsumption > _finalMemoryThreshold)
			{
				optional<SymbolCount> minimalThreshold = compactificator.GetMinimalCount();
				if (minimalThreshold)
					_memoryConsumption -= compactificator.Compactify(*minimalThreshold);
			}
			Normalize();
			shared_ptr<Model> model;
			model.swap(_model);
			_context.clear();
			return make_shared<Dictionary>(model);
		}

		std::string ToString() const
		{ return StringBuilder() % _model; }

	private:
		void AddSymbol(Symbol symbol)
		{
			_memoryConsumption += _model->AddSymbol(_context, symbol, SymbolCount(1));
			UpdateContext(symbol);

			if (_memoryConsumption > _highMemoryThreshold)
				Compactify();
		}

		void UpdateContext(Symbol symbol)
		{
			_context.push_back(symbol);
			if (_context.size() > Model::ContextSize)
				_context.pop_front();
		}

		void Compactify()
		{
			typename Model::Compactificator compactificator(*_model);

			while (_memoryConsumption > _lowMemoryThreshold)
			{
				optional<SymbolCount> minimalThreshold = compactificator.GetMinimalCount();
				if (minimalThreshold)
					_memoryConsumption -= compactificator.Compactify(*minimalThreshold);
			}
		}

		void Normalize()
		{ _model->Normalize(); }
	};


}

#endif
