#ifndef STINGRAY_TOOLKIT_PAIRVALUECOMPARER_H
#define STINGRAY_TOOLKIT_PAIRVALUECOMPARER_H

namespace stingray
{

	template < typename Comparer, typename ValueExtractor >
	class PairValueComparer
	{
	private:
		Comparer	_comparer;

	public:
		PairValueComparer(Comparer comparer)
			: _comparer(comparer)
		{ }

		template < typename Pair >
		bool operator() (const Pair& x, const Pair& y) const
		{ return _comparer(ValueExtractor::GetValue(x), ValueExtractor::GetValue(y)); }
	};

	struct FirstPairValueExtractor
	{
		template < typename Pair >
		static typename Pair::first_type GetValue(const Pair& pair)
		{ return pair.first; }
	};

	struct SecondPairValueExtractor
	{
		template < typename Pair >
		static typename Pair::second_type GetValue(const Pair& pair)
		{ return pair.second; }
	};

	template < typename Comparer >
	PairValueComparer<Comparer, FirstPairValueExtractor> CompareFirstPairValue(Comparer comparer)
	{ return PairValueComparer<Comparer, FirstPairValueExtractor>(comparer); }

	template < typename Comparer >
	PairValueComparer<Comparer, SecondPairValueExtractor> CompareSecondPairValue(Comparer comparer)
	{ return PairValueComparer<Comparer, SecondPairValueExtractor>(comparer); }

}

#endif
