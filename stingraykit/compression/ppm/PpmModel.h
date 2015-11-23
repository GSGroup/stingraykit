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

	template <typename PpmConfig_, typename ModelConfigList_, typename Enabler = void>
	class CompactificatorTemplate;


	template <typename PpmConfig_, typename ModelConfigList_>
	class PpmModel
	{
	public:
		typedef typename PpmConfig_::Symbol       Symbol;
		typedef typename PpmConfig_::SymbolCount  SymbolCount;
		typedef typename ModelConfigList_::ValueT ModelConfig;

		static const size_t ContextSize = ModelConfig::ContextSize;

		struct SymbolCounter
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

			std::string ToString() const     { return StringBuilder() % GetSymbol() % ": " % GetCount(); }
		};
		typedef std::vector<SymbolCounter> Symbols;


		class PpmContextBuilder
		{
		private:
			SymbolCount _exit;
			Symbols     _symbols;

		public:
			PpmContextBuilder() : _exit(0)
			{ }

			size_t AddSymbol(Symbol symbol, SymbolCount count)
			{
				typename Symbols::iterator it = std::lower_bound(_symbols.begin(), _symbols.end(), SymbolCounter(symbol, 0), CompareMembersLess(&SymbolCounter::GetSymbol));
				if (it == _symbols.end() || it->GetSymbol() != symbol)
				{
					_symbols.insert(it, SymbolCounter(symbol, count));
					return sizeof(SymbolCounter);
				}
				it->SetCount(it->GetCount() + count);
				return 0;
			}

			SymbolCount GetTotalCount() const
			{ return Sum(Transform(ToRange(_symbols), bind(&SymbolCounter::GetCount, _1))); }

			const Symbols& GetSymbols() const
			{ return _symbols; }

			size_t GetTotalSize() const
			{ return _symbols.size() * sizeof(SymbolCounter); }

			void Normalize()
			{
				SymbolCount maxCount = std::max(*Range::MaxElement(Transform(ToRange(_symbols), bind(&SymbolCounter::GetCount, _1)), comparers::Less()), _exit);
				if (maxCount > ModelConfig::Scale)
				{
					for (typename Symbols::iterator it = _symbols.begin(); it != _symbols.end(); ++it)
						it->SetCount((u64)it->GetCount() * ModelConfig::Scale / maxCount);
					_exit = std::max(SymbolCount(1), SymbolCount(_exit * ModelConfig::Scale / maxCount));
				}
				else
					_exit = std::max(SymbolCount(1), _exit);

				if (ContextSize == 0)
				{
					Symbols replacement;
					typename Symbols::iterator it = _symbols.begin();
					Symbol s = IntTraits<Symbol>::Min;
					do
					{
						if (it != _symbols.end() && it->GetSymbol() == s && it->GetCount() != 0)
							replacement.push_back(*it++);
						else
							replacement.push_back(SymbolCounter(s, 1));
					}
					while (s++ != IntTraits<Symbol>::Max);
					_symbols.swap(replacement);
				}
				else
				{
					Symbols replacement;
					Copy(Filter(ToRange(_symbols), bind(&SymbolCounter::GetCount, _1)), std::back_inserter(replacement));
					_symbols.swap(replacement);
				}
			}

			template <typename Consumer_>
			bool Predict(optional<Symbol> symbol, const Consumer_& f) const
			{
				SymbolCount scale = Sum(Transform(ToRange(_symbols), bind(&SymbolCounter::GetCount, _1))) + _exit;
				if (!symbol)
				{
					f(scale - _exit, scale, scale);
					return false;
				}

				typename Symbols::const_iterator it = std::lower_bound(_symbols.begin(), _symbols.end(), SymbolCounter(*symbol, 0), CompareMembersLess(&SymbolCounter::GetSymbol));
				if (it == _symbols.end() || it->GetSymbol() != *symbol)
				{
					f(scale - _exit, scale, scale);
					return false;
				}
				SymbolCount low = Sum(Transform(ToRange(_symbols.begin(), it), bind(&SymbolCounter::GetCount, _1)));
				f(low, low + it->GetCount(), scale);
				return true;
			}

			template <typename GetBitFunctor_>
			optional<Symbol> Decode(ArithmeticDecoder& decoder, const GetBitFunctor_& getBit) const
			{
				SymbolCount scale = Sum(Transform(ToRange(_symbols), bind(&SymbolCounter::GetCount, _1))) + _exit;
				u32 low = 0;
				u32 targetProbability = decoder.GetProbability(scale);
				for (typename Symbols::const_iterator it = _symbols.begin(); it != _symbols.end(); ++it)
				{
					u32 high = low + it->GetCount();
					if (low <= targetProbability && targetProbability < high)
					{
						decoder.SymbolDecoded(low, high, scale, getBit);
						return it->GetSymbol();
					}
					low = high;
				}
				decoder.SymbolDecoded(scale - _exit, scale, scale, getBit);
				return null;
			}

			std::string ToString() const
			{ return StringBuilder() % _symbols; }
		};


	public:
		typedef std::map<array<Symbol, ContextSize>, PpmContextBuilder> Contexts;
		typedef PpmModel<PpmConfig_, typename ModelConfigList_::Next>   ChildModel;
		typedef CompactificatorTemplate<PpmConfig_, ModelConfigList_>   Compactificator;

		static const size_t EntryMemorySize = sizeof(typename Contexts::value_type) + sizeof(size_t) * 3;

	private:
		Contexts   _contexts;
		ChildModel _childModel;

	public:
		PpmModel()
		{ CompileTimeAssert<(ContextSize > ChildModel::ContextSize || SameType<typename ModelConfigList_::Next, TypeListEndNode>::Value)> ModelConfigsMustBeSortedInDescendingOrder; }

		size_t AddSymbol(const std::deque<Symbol>& context, Symbol symbol, SymbolCount count)
		{
			if (context.size() < ContextSize)
				return _childModel.AddSymbol(context, symbol, count);

			array<Symbol, ContextSize> ctx;
			std::copy(context.end() - ContextSize, context.end(), ctx.begin());
			return DoAddSymbol(ctx, symbol, count);
		}

		size_t DoAddSymbol(array<Symbol, ContextSize> context, Symbol symbol, SymbolCount count)
		{
			std::pair<typename Contexts::iterator, bool> p = _contexts.insert(std::make_pair(context, PpmContextBuilder()));
			return p.first->second.AddSymbol(symbol, count) + (p.second ? EntryMemorySize : 0);
		}

		void Normalize()
		{
			for (typename Contexts::iterator it = _contexts.begin(); it != _contexts.end(); ++it)
				it->second.Normalize();
			_childModel.Normalize();
		}

		Contexts& GetContexts()     { return _contexts; }
		ChildModel& GetChildModel() { return _childModel; }

		template <typename Consumer_>
		void Predict(const std::deque<Symbol>& context, optional<Symbol> symbol, const Consumer_& f) const
		{
			if (context.size() < ContextSize)
				return _childModel.Predict(context, symbol, f);

			array<Symbol, ContextSize> ctx;
			std::copy(context.end() - ContextSize, context.end(), ctx.begin());
			typename Contexts::const_iterator it = _contexts.find(ctx);
			if (it == _contexts.end() || !it->second.Predict(symbol, f))
				_childModel.Predict(context, symbol, f);
		}

		template <typename GetBitFunctor_>
		optional<Symbol> Decode(const std::deque<Symbol>& context, ArithmeticDecoder& d, const GetBitFunctor_& f) const
		{
			if (context.size() < ContextSize)
				return _childModel.Decode(context, d, f);

			array<Symbol, ContextSize> ctx;
			std::copy(context.end() - ContextSize, context.end(), ctx.begin());
			typename Contexts::const_iterator it = _contexts.find(ctx);
			if (it == _contexts.end())
				return _childModel.Decode(context, d, f);
			optional<Symbol> s = it->second.Decode(d, f);
			return s ? *s : _childModel.Decode(context, d, f);
		}

		std::string ToString() const
		{ return StringBuilder() % "PpmModel { order: " % ContextSize % ", contexts: " % _contexts % " }, child: { " % _childModel % " }"; }
	};


	template <typename PpmConfig_>
	class PpmModel<PpmConfig_, TypeListEndNode>
	{
	public:
		typedef typename PpmConfig_::Symbol       Symbol;
		typedef typename PpmConfig_::SymbolCount  SymbolCount;
		static const size_t ContextSize = 0;

		size_t AddSymbol(const std::deque<Symbol>& context, Symbol symbol, SymbolCount count)
		{ return 0; }

		void Normalize()
		{ }

		template <typename Consumer_>
		void Predict(const std::deque<Symbol>& context, optional<Symbol> symbol, const Consumer_& f) const
		{ STINGRAYKIT_CHECK(!symbol, "Should get here at EOF only!"); }

		template <typename GetBitFunctor_>
		optional<Symbol> Decode(const std::deque<Symbol>& context, ArithmeticDecoder& d, const GetBitFunctor_& f) const
		{ return null; }

		std::string ToString() const
		{ return "{ }"; }
	};


	template <typename PpmConfig_, typename ModelConfigList_, typename Enabler>
	class CompactificatorTemplate
	{
	public:
		typedef typename PpmConfig_::Symbol                                          Symbol;
		typedef typename PpmConfig_::SymbolCount                                     SymbolCount;
		typedef PpmModel<PpmConfig_, ModelConfigList_>                               Model;
		typedef typename Model::Symbols                                              Symbols;
		typedef typename Model::Contexts                                             Contexts;
		typedef CompactificatorTemplate<PpmConfig_, typename ModelConfigList_::Next> ChildCompactificator;

		static const size_t ContextSize = Model::ContextSize;

	private:
		class Entry
		{
		private:
			SymbolCount                 _count;
			typename Contexts::iterator _it;

		public:
			Entry(typename Contexts::iterator it) :
				_count(it->second.GetTotalCount()), _it(it)
			{ }

			SymbolCount GetCount() const                         { return _count; }
			void SetCount(SymbolCount c)                         { _count = c; }
			const array<Symbol, ContextSize>& GetContext() const { return _it->first; }
			const Symbols& GetSymbols() const                    { return _it->second.GetSymbols(); }

			typename Contexts::iterator GetIt() const            { return _it; }

			std::string ToString() const                         { return StringBuilder() % "{ context: " % GetContext() % ", count: " % GetCount() % " }"; }
		};
		typedef std::vector<Entry> Entries;

	private:
		Model&               _model;
		Entries              _entries;
		ChildCompactificator _childCompacificator;

	public:
		CompactificatorTemplate(Model& model) : _model(model), _childCompacificator(model.GetChildModel())
		{
			for (typename Contexts::iterator it = _model.GetContexts().begin(); it != _model.GetContexts().end(); ++it)
				_entries.push_back(it);
			std::sort(_entries.begin(), _entries.end(), CompareMembersGreater(&Entry::GetCount));
		}

		size_t AddSymbol(array<Symbol, ContextSize> context, Symbol symbol, SymbolCount count)
		{
			std::pair<typename Contexts::iterator, bool> p = _model.GetContexts().insert(std::make_pair(context, typename Model::PpmContextBuilder()));
			if (!p.second)
			{
				Entry e(p.first);
				std::pair<typename Entries::iterator, typename Entries::iterator> range = std::equal_range(_entries.begin(), _entries.end(), e, CompareMembersGreater(&Entry::GetCount));
				typename Entries::iterator it = std::find_if(range.first, range.second, bind(CompareMembersEquals(&Entry::GetContext), e, _1));
				STINGRAYKIT_CHECK(it != range.second, StringBuilder() % "Can't find context! Entries: " % _entries % ", e: " % e);
				_entries.erase(it);
			}

			size_t result = p.first->second.AddSymbol(symbol, count) + (p.second ? Model::EntryMemorySize : 0);
			DoAdd(p.first);
			return result;
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

		size_t Compactify(SymbolCount minimalThreshold)
		{
			size_t freedMemory = 0;
			while (!_entries.empty() && _entries.back().GetCount() <= minimalThreshold)
				freedMemory += DoPop();
			return freedMemory + _childCompacificator.Compactify(minimalThreshold);
		}

	private:
		void DoAdd(typename Contexts::iterator it)
		{
			Entry e(it);
			typename Entries::iterator insertionPoint = std::lower_bound(_entries.begin(), _entries.end(), e, CompareMembersGreater(&Entry::GetCount));
			_entries.insert(insertionPoint, e);
		}

		size_t DoPop()
		{
			Entry e = _entries.back();

			CompileTimeAssert<(ContextSize > ChildCompactificator::ContextSize)> ErrorContextSizeMismatch;
			array<Symbol, ChildCompactificator::ContextSize> recipientContext;
			std::copy(e.GetContext().end() - ChildCompactificator::ContextSize, e.GetContext().end(), recipientContext.begin());

			size_t consumedMemory = 0;
			for (typename Symbols::const_iterator it = e.GetSymbols().begin(); it != e.GetSymbols().end(); ++it)
				consumedMemory += _childCompacificator.AddSymbol(recipientContext, it->GetSymbol(), it->GetCount());

			typename Contexts::iterator it = e.GetIt();
			size_t freedMemory = it->second.GetTotalSize() + Model::EntryMemorySize;
			_model.GetContexts().erase(it);
			_entries.pop_back();

			STINGRAYKIT_CHECK(freedMemory >= consumedMemory, "Popping contexts should free memory, not consume it!");
			return freedMemory - consumedMemory;
		}
	};


	template <typename PpmConfig_, typename ModelConfigList_>
	class CompactificatorTemplate<PpmConfig_, ModelConfigList_, typename EnableIf<GetTypeListLength<ModelConfigList_>::Value == 1, void>::ValueT>
	{
	public:
		typedef typename PpmConfig_::Symbol            Symbol;
		typedef typename PpmConfig_::SymbolCount       SymbolCount;
		typedef PpmModel<PpmConfig_, ModelConfigList_> Model;

		static const size_t ContextSize = Model::ContextSize;

	private:
		Model& _model;

	public:
		CompactificatorTemplate(Model& model) : _model(model)
		{ }

		size_t AddSymbol(array<Symbol, Model::ContextSize> context, Symbol symbol, SymbolCount count)
		{ return _model.DoAddSymbol(context, symbol, count); }

		optional<SymbolCount> GetMinimalCount() const
		{ return null; }

		size_t Compactify(SymbolCount minimalThreshold)
		{ return 0; }
	};

}

#endif
