#ifndef STINGRAYKIT_STRING_COMPARERS_H
#define STINGRAYKIT_STRING_COMPARERS_H

#include <stingraykit/compare/comparers.h>

namespace stingray
{

	namespace Detail
	{

		template < class Comparer >
		class CaseInsensitiveCompare
		{
		private:
			Comparer	_comparer;

		public:
			explicit CaseInsensitiveCompare(Comparer comparer = Comparer())
				: _comparer(comparer)
			{ }

			bool operator()(char first, char second) const
			{
				return _comparer(std::tolower(first), std::tolower(second));
			}
		};

	}


	class CaseInsensitiveLess
	{
	public:
		bool operator()(const std::string& first, const std::string& second) const
		{
			return std::lexicographical_compare(first.begin(), first.end(), second.begin(), second.end(), Detail::CaseInsensitiveCompare<comparers::Less>());
		}
	};


	class CaseInsensitiveEquals
	{
	public:
		bool operator()(const std::string& first, const std::string& second) const
		{
			return first.length() == second.length() && std::equal(first.begin(), first.end(), second.begin(), Detail::CaseInsensitiveCompare<comparers::Equals>());
		}
	};

}

#endif
