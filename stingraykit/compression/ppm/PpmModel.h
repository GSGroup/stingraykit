#ifndef STINGRAYKIT_COMPRESSION_PPM_PPMMODEL_H
#define STINGRAYKIT_COMPRESSION_PPM_PPMMODEL_H

#include <stingraykit/collection/Range.h>
#include <stingraykit/compression/ArithmeticCoder.h>
#include <stingraykit/compression/ArithmeticDecoder.h>
#include <stingraykit/exception.h>
#include <stingraykit/function/functional.h>

#include <algorithm>
#include <deque>
#include <map>
#include <string.h>
#include <vector>

namespace stingray
{

	template <typename PpmConfig_>
	struct PpmImpl
	{
		typedef typename PpmConfig_::Symbol                          Symbol;
		typedef typename PpmConfig_::SymbolCount                     SymbolCount;


		class SymbolCounter
		{
		private:
			Symbol      _symbol;
			SymbolCount _count;

		public:
			SymbolCounter(Symbol symbol, SymbolCount count) :
				_symbol(symbol), _count(count)
			{ }

			Symbol GetSymbol() const         { return _symbol; }

			SymbolCount GetCount() const     { return _count; }
			void SetCount(SymbolCount count) { _count = count; }
		};


		class ContextInfoBuilder
		{
		public:
			typedef std::vector<SymbolCounter> Symbols;

		private:
			SymbolCount _exit;
			Symbols     _symbols;

		public:
			ContextInfoBuilder() : _exit(0)
			{ }

			bool AddSymbol(Symbol symbol, SymbolCount count)
			{
				typename Symbols::iterator it = std::lower_bound(_symbols.begin(), _symbols.end(), symbol, CompareMemberLess(&SymbolCounter::GetSymbol));
				if (it == _symbols.end() || it->GetSymbol() != symbol)
				{
					_symbols.insert(it, SymbolCounter(symbol, count));
					return true;
				}
				it->SetCount(it->GetCount() + count);
				return false;
			}

			SymbolCount GetTotalCount() const
			{ return Sum(Transform(ToRange(_symbols), Bind(&SymbolCounter::GetCount, _1))); }

			const Symbols& GetSymbols() const
			{ return _symbols; }

			SymbolCount GetExit() const
			{ return _exit; }
		};


		template <typename ModelConfigList_, typename Enabler = void>
		class ModelBuilder
		{
		public:
			typedef typename ModelConfigList_::ValueT             ModelConfig;
			typedef ModelBuilder<typename ModelConfigList_::Next> ChildModel;
			typedef typename ModelConfig::Probability             Probability;
			typedef array<Symbol, ModelConfig::ContextSize>       Context;
			typedef std::map<Context, ContextInfoBuilder>         ContextBuilders;

			static const size_t ContextSize = ModelConfig::ContextSize;
			static const size_t ContextInfoBuilderMemorySize = sizeof(typename ContextBuilders::value_type) + sizeof(size_t) * 3;

		private:
			ContextBuilders _contexts;
			ChildModel      _childModel;

			size_t          _contextsCount;
			size_t          _entriesCount;

		public:
			ModelBuilder() : _contextsCount(0), _entriesCount(0)
			{ static_assert((ContextSize > ChildModel::ContextSize || IsSame<typename ModelConfigList_::Next, TypeListEndNode>::Value), "Model configs must be sorted in descending order"); }

			void AddSymbol(const std::deque<Symbol>& context, Symbol symbol, SymbolCount count)
			{
				if (context.size() < ContextSize)
				{
					_childModel.AddSymbol(context, symbol, count);
					return;
				}

				array<Symbol, ContextSize> ctx;
				std::copy(context.end() - ContextSize, context.end(), ctx.begin());
				AddSymbol(ctx, symbol, count);
			}

			void AddSymbol(array<Symbol, ContextSize> context, Symbol symbol, SymbolCount count)
			{ DoAddSymbol(AddContext(context).first, symbol, count); }

			std::pair<typename ContextBuilders::iterator, bool> AddContext(array<Symbol, ContextSize> context)
			{
				std::pair<typename ContextBuilders::iterator, bool> p = _contexts.insert(std::make_pair(context, ContextInfoBuilder()));
				if (p.second)
					++_contextsCount;
				return p;
			}

			void DoAddSymbol(typename ContextBuilders::iterator it, Symbol symbol, SymbolCount count)
			{
				if (it->second.AddSymbol(symbol, count))
					++_entriesCount;
			}

			void RemoveContext(typename ContextBuilders::iterator it)
			{
				_entriesCount -= it->second.GetSymbols().size();
				--_contextsCount;
				_contexts.erase(it);
			}

			ContextBuilders& GetContexts() { return _contexts; }
			ChildModel& GetChildModel()    { return _childModel; }

			template <typename Counter_>
			size_t GetMemoryConsumption(const Counter_& c) const
			{ return c(this, _contextsCount, _entriesCount) + _childModel.GetMemoryConsumption(c); }
		};


		template <typename ModelConfigList_>
		class ModelBuilder<ModelConfigList_, typename EnableIf<GetTypeListLength<ModelConfigList_>::Value == 0, void>::ValueT>
		{
		public:
			static const size_t ContextSize = 0;

			void AddSymbol(const std::deque<Symbol>& context, Symbol symbol, SymbolCount count)
			{ }

			template <typename Counter_>
			size_t GetMemoryConsumption(const Counter_& c) const
			{ return 0; }
		};


		struct ModelBuilderMemoryCounter
		{
			template <typename ModelConfigList_>
			size_t operator() (const ModelBuilder<ModelConfigList_>*, size_t contextsCount, size_t entriesCount) const
			{ return contextsCount * ModelBuilder<ModelConfigList_>::ContextInfoBuilderMemorySize + entriesCount * sizeof(SymbolCounter); }
		};


		template <typename ModelConfigList_, typename Enabler = void>
		class Compactificator
		{
		public:
			typedef ModelBuilder<ModelConfigList_>                   Model;
			typedef array<Symbol, Model::ContextSize>                Context;
			typedef std::map<Context, ContextInfoBuilder>            ContextBuilders;
			typedef typename ContextInfoBuilder::Symbols             Symbols;
			typedef Compactificator<typename ModelConfigList_::Next> ChildCompactificator;

			static const size_t ContextSize = Model::ContextSize;

		private:
			class Entry
			{
			private:
				SymbolCount                        _count;
				typename ContextBuilders::iterator _it;

			public:
				Entry(typename ContextBuilders::iterator it) :
					_count(it->second.GetTotalCount()), _it(it)
				{ }

				SymbolCount GetCount() const                     { return _count; }
				typename ContextBuilders::iterator GetIt() const { return _it; }

				const Context& GetContext() const                { return _it->first; }
				const Symbols& GetSymbols() const                { return _it->second.GetSymbols(); }
			};
			typedef std::vector<Entry> Entries;

		private:
			Model&               _model;
			Entries              _entries;
			ChildCompactificator _childCompacificator;

		public:
			Compactificator(Model& model) : _model(model), _childCompacificator(model.GetChildModel())
			{
				for (typename ContextBuilders::iterator it = _model.GetContexts().begin(); it != _model.GetContexts().end(); ++it)
					_entries.push_back(it);
				std::sort(_entries.begin(), _entries.end(), CompareMembersGreater(&Entry::GetCount));
			}

			void AddSymbol(array<Symbol, ContextSize> context, Symbol symbol, SymbolCount count)
			{
				std::pair<typename ContextBuilders::iterator, bool> p = _model.AddContext(context);
				if (!p.second)
				{
					Entry e(p.first);
					std::pair<typename Entries::iterator, typename Entries::iterator> range = std::equal_range(_entries.begin(), _entries.end(), e, CompareMembersGreater(&Entry::GetCount));
					typename Entries::iterator it = std::find_if(range.first, range.second, Bind(CompareMembersEquals(&Entry::GetContext), e, _1));
					STINGRAYKIT_CHECK(it != range.second, "Can't find context!");
					_entries.erase(it);
				}

				_model.DoAddSymbol(p.first, symbol, count);

				Entry e(p.first);
				typename Entries::iterator insertionPoint = std::lower_bound(_entries.begin(), _entries.end(), e, CompareMembersGreater(&Entry::GetCount));
				_entries.insert(insertionPoint, e);
			}

			optional<SymbolCount> GetMinimalCount() const
			{
				optional<SymbolCount> myMinimal = _entries.empty() ? optional<SymbolCount>() : _entries.back().GetCount();
				optional<SymbolCount> childMinimal = _childCompacificator.GetMinimalCount();
				if (!myMinimal)
					return childMinimal;
				else if (!childMinimal)
					return myMinimal;
				else
					return std::min(*myMinimal, *childMinimal);
			}

			void Compactify(SymbolCount minimalThreshold)
			{
				while (!_entries.empty() && _entries.back().GetCount() <= minimalThreshold)
					DoPop();
				_childCompacificator.Compactify(minimalThreshold);
			}

		private:
			void DoPop()
			{
				Entry e = _entries.back();

				static_assert((ContextSize > ChildCompactificator::ContextSize), "Context size mismatch");
				array<Symbol, ChildCompactificator::ContextSize> recipientContext;
				std::copy(e.GetContext().end() - ChildCompactificator::ContextSize, e.GetContext().end(), recipientContext.begin());

				for (typename Symbols::const_iterator it = e.GetSymbols().begin(); it != e.GetSymbols().end(); ++it)
					_childCompacificator.AddSymbol(recipientContext, it->GetSymbol(), it->GetCount());

				_model.RemoveContext(e.GetIt());
				_entries.pop_back();
			}
		};


		template <typename ModelConfigList_>
		class Compactificator<ModelConfigList_, typename EnableIf<GetTypeListLength<ModelConfigList_>::Value == 1, void>::ValueT>
		{
		public:
			typedef ModelBuilder<ModelConfigList_> Model;
			static const size_t ContextSize = Model::ContextSize;

		private:
			Model& _model;

		public:
			Compactificator(Model& model) : _model(model)
			{ }

			void AddSymbol(array<Symbol, Model::ContextSize> context, Symbol symbol, SymbolCount count)
			{ _model.AddSymbol(context, symbol, count); }

			optional<SymbolCount> GetMinimalCount() const
			{ return null; }

			size_t Compactify(SymbolCount minimalThreshold)
			{ return 0; }
		};


		template < typename ModelConfigList_, size_t ConfigListSize = GetTypeListLength<ModelConfigList_>::Value >
		class Model
		{
		public:
			typedef ModelBuilder<ModelConfigList_>             Builder;
			typedef typename ModelConfigList_::ValueT          ModelConfig;
			typedef Model<typename ModelConfigList_::Next>     ChildModel;
			typedef typename ModelConfig::Probability          Probability;
			typedef array<Symbol, ModelConfig::ContextSize>    Context;

			static const size_t ContextSize = ModelConfig::ContextSize;

		private:
			class SymbolProbability
			{
			private:
				Symbol      _symbol;
				Probability _probability;

			public:
				SymbolProbability(Symbol symbol, Probability probability) :
					_symbol(symbol), _probability(probability)
				{ }

				Symbol GetSymbol() const           { return _symbol; }
				Probability GetProbability() const { return _probability; }

				std::string ToString() const
				{ return StringBuilder() % _symbol % " => " % _probability; }
			};
			typedef std::vector<SymbolProbability> Symbols;

			class ContextInfo
			{
			private:
				Context     _context;
				Probability _exitProbability;
				size_t      _offset;

			public:
				ContextInfo(const Context& context, Probability exitProbability, size_t offset) :
					_context(context), _exitProbability(exitProbability), _offset(offset)
				{ }

				const Context& GetContext() const             { return _context; }
				const Probability& GetExitProbability() const { return _exitProbability; }
				size_t GetOffset() const               { return _offset; }
			};
			typedef std::vector<ContextInfo> Contexts;

		public:
			static const size_t ContextInfoMemorySize = sizeof(ContextInfo);
			static const size_t SymbolProbabilityMemorySize = sizeof(SymbolProbability);

		private:
			Symbols    _symbols;
			Contexts   _contexts;
			ChildModel _childModel;

		public:
			Model(ModelBuilder<ModelConfigList_>& modelBuilder) : _childModel(modelBuilder.GetChildModel())
			{
				for (typename Builder::ContextBuilders::iterator it = modelBuilder.GetContexts().begin(); it != modelBuilder.GetContexts().end(); ++it)
				{
					size_t oldOffset = _symbols.size();

					ContextInfoBuilder& contextBuilder(it->second);
					SymbolCount maxCount = std::max(*Range::MaxElement(Transform(ToRange(contextBuilder.GetSymbols()), Bind(&SymbolCounter::GetCount, _1))), contextBuilder.GetExit());
					if (ContextSize != 0)
					{
						for (typename ContextInfoBuilder::Symbols::const_iterator s = contextBuilder.GetSymbols().begin(); s != contextBuilder.GetSymbols().end(); ++s)
						{
							Probability p = maxCount > ModelConfig::Scale ? (u64)s->GetCount() * ModelConfig::Scale / maxCount : s->GetCount();
							if (p != 0)
								_symbols.push_back(SymbolProbability(s->GetSymbol(), p));
						}
					}
					else
					{
						typename ContextInfoBuilder::Symbols::const_iterator s = contextBuilder.GetSymbols().begin();
						for (s64 symbol = IntTraits<Symbol>::Min; symbol != (s64)IntTraits<Symbol>::Max + 1; ++symbol)
						{
							if (s == contextBuilder.GetSymbols().end() || s->GetSymbol() != symbol)
								_symbols.push_back(SymbolProbability(symbol, Probability(1)));
							else
							{
								Probability p = maxCount > ModelConfig::Scale ? (u64)s->GetCount() * ModelConfig::Scale / maxCount : s->GetCount();
								_symbols.push_back(SymbolProbability(symbol, std::max(p, Probability(1))));
								s++;
							}
						}
					}

					if (oldOffset == _symbols.size())
						continue;

					Probability exit = std::max(SymbolCount(1), SymbolCount(maxCount > ModelConfig::Scale ? contextBuilder.GetExit() * ModelConfig::Scale / maxCount : contextBuilder.GetExit()));
					_contexts.push_back(ContextInfo(it->first, exit, oldOffset));
				}
			}

			template <typename Consumer_>
			void Predict(const std::deque<Symbol>& context, optional<Symbol> symbol, const Consumer_& f) const
			{
				if (context.size() < ContextSize)
				{
					_childModel.Predict(context, symbol, f);
					return;
				}

				Context ctx;
				std::copy(context.end() - ContextSize, context.end(), ctx.begin());
				typename Contexts::const_iterator it = std::lower_bound(_contexts.begin(), _contexts.end(), ctx, CompareMemberLess(&ContextInfo::GetContext));
				if (it == _contexts.end())
				{
					_childModel.Predict(context, symbol, f);
					return;
				}

				typename Symbols::const_iterator first = _symbols.begin() + it->GetOffset();
				typename Symbols::const_iterator last = (std::next(it) != _contexts.end()) ? (_symbols.begin() + std::next(it)->GetOffset()) : _symbols.end();
				if (!DoPredict(first, last, it->GetExitProbability(), symbol, f))
					_childModel.Predict(context, symbol, f);
			}

			template <typename GetBitFunctor_>
			optional<Symbol> Decode(const std::deque<Symbol>& context, ArithmeticDecoder& d, const GetBitFunctor_& f) const
			{
				if (context.size() < ContextSize)
					return _childModel.Decode(context, d, f);

				Context ctx;
				std::copy(context.end() - ContextSize, context.end(), ctx.begin());
				typename Contexts::const_iterator it = std::lower_bound(_contexts.begin(), _contexts.end(), ctx, CompareMemberLess(&ContextInfo::GetContext));
				if (it == _contexts.end())
					return _childModel.Decode(context, d, f);

				typename Symbols::const_iterator first = _symbols.begin() + it->GetOffset();
				typename Symbols::const_iterator last = (std::next(it) != _contexts.end()) ? (_symbols.begin() + std::next(it)->GetOffset()) : _symbols.end();
				optional<Symbol> s = DoDecode(first, last, it->GetExitProbability(), d, f);
				return s ? *s : _childModel.Decode(context, d, f);
			}

			std::string ToString() const
			{
				StringBuilder result;
				result % _childModel;
				for (typename Contexts::const_iterator it = _contexts.begin(); it != _contexts.end(); ++it)
				{
					typename Symbols::const_iterator first = _symbols.begin() + it->GetOffset();
					typename Symbols::const_iterator last = (std::next(it) != _contexts.end()) ? (_symbols.begin() + std::next(it)->GetOffset()) : _symbols.end();
					result % "Context: " % it->GetContext() % ", exit: " % it->GetExitProbability() % ", symbols: " % ToRange(first, last) % "\n";
				}
				return result;
			}

		private:
			template <typename Consumer_>
			bool DoPredict(typename Symbols::const_iterator first, typename Symbols::const_iterator last, Probability exit, optional<Symbol> symbol, const Consumer_& f) const
			{
				SymbolCount scale = Sum(Transform(Transform(ToRange(first, last), Bind(&SymbolProbability::GetProbability, _1)), ImplicitCaster<SymbolCount>())) + exit;
				if (!symbol)
				{
					f(scale - exit, scale, scale);
					return false;
				}

				typename Symbols::const_iterator it = std::lower_bound(first, last, *symbol, CompareMemberLess(&SymbolProbability::GetSymbol));
				if (it == last || it->GetSymbol() != *symbol)
				{
					f(scale - exit, scale, scale);
					return false;
				}
				SymbolCount low = Sum(Transform(ToRange(first, it), Bind(&SymbolProbability::GetProbability, _1)));
				f(low, low + it->GetProbability(), scale);
				return true;
			}

			template <typename GetBitFunctor_>
			optional<Symbol> DoDecode(typename Symbols::const_iterator first, typename Symbols::const_iterator last, Probability exit, ArithmeticDecoder& decoder, const GetBitFunctor_& getBit) const
			{
				SymbolCount scale = Sum(Transform(Transform(ToRange(first, last), Bind(&SymbolProbability::GetProbability, _1)), ImplicitCaster<SymbolCount>())) + exit;
				u32 low = 0;
				u32 targetProbability = decoder.GetProbability(scale);
				for (typename Symbols::const_iterator it = first; it != last; ++it)
				{
					u32 high = low + it->GetProbability();
					if (low <= targetProbability && targetProbability < high)
					{
						decoder.SymbolDecoded(low, high, scale, getBit);
						return it->GetSymbol();
					}
					low = high;
				}
				decoder.SymbolDecoded(scale - exit, scale, scale, getBit);
				return null;
			}
		};


		template < typename ModelConfigList_ >
		class Model<ModelConfigList_, 0>
		{
		public:
			Model(ModelBuilder<ModelConfigList_>& modelBuilder)
			{ }

			template <typename Consumer_>
			void Predict(const std::deque<Symbol>& context, optional<Symbol> symbol, const Consumer_& f) const
			{ STINGRAYKIT_CHECK(!symbol, "Should get here at EOF only!"); }

			template <typename GetBitFunctor_>
			optional<Symbol> Decode(const std::deque<Symbol>& context, ArithmeticDecoder& d, const GetBitFunctor_& f) const
			{ return null; }

			std::string ToString() const
			{ return ""; }
		};


		struct ModelMemoryCounter
		{
			template <typename ModelConfigList_>
			size_t operator() (const ModelBuilder<ModelConfigList_>*, size_t contextsCount, size_t entriesCount) const
			{ return contextsCount * Model<ModelConfigList_>::ContextInfoMemorySize + entriesCount * Model<ModelConfigList_>::SymbolProbabilityMemorySize; }
		};
	};

}

#endif
