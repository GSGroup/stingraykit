#ifndef STINGRAYKIT_STRING_STRINGUTILS_H
#define STINGRAYKIT_STRING_STRINGUTILS_H

// Copyright (c) 2011 - 2025, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/string/lexical_cast.h>

#include <algorithm>
#include <ctype.h>

namespace stingray
{

	template < typename CharType >
	std::basic_string<CharType> ReplaceAll(
			basic_string_view<CharType> str,
			basic_string_view<CharType> from,
			basic_string_view<CharType> to,
			Dummy dummy = Dummy())
	{
		using StringType = std::basic_string<CharType>;
		using StringViewType = basic_string_view<CharType>;

		typename std::common_type<typename StringType::size_type, typename StringViewType::size_type>::type index = str.find(from);
		if (index == StringViewType::npos)
			return str.copy();

		StringType result(str.copy());
		do
		{
			result.replace(std::next(result.begin(), index), std::next(result.begin(), index + from.size()), to.begin(), to.end());
			index = std::distance(result.begin(), std::search(std::next(result.begin(), index + to.size()), result.end(), from.begin(), from.end()));
		}
		while (index < result.size());

		return result;
	}


	inline std::string ReplaceAll(string_view str, string_view from, string_view to)
	{ return ReplaceAll<char>(str, from, to); }


	inline std::string ReplaceAll(string_view str, char from, char to)
	{
		const string_view::size_type index = str.find(from);
		if (index == string_view::npos)
			return str.copy();

		std::string result(str.copy());
		std::replace(std::next(result.begin(), index), result.end(), from, to);
		return result;
	}


	inline std::string Filter(string_view str, string_view characters)
	{
		std::string result;
		for (char ch : str)
			if (characters.find(ch) == string_view::npos)
				result.push_back(ch);
		return result;
	}


	inline std::string Remove(string_view str, char ch = ' ')
	{
		std::string result(str.copy());
		result.erase(std::remove(result.begin(), result.end(), ch), result.end());
		return result;
	}


	inline string_view ExtractPrefix(string_view str, size_t prefixLength)
	{ return str.substr(0, prefixLength); }


	inline string_view ExtractSuffix(string_view str, size_t suffixLength)
	{
		const size_t length = std::min(str.length(), suffixLength);
		return str.substr(str.length() - length, length);
	}


	inline string_view ExtractCommonPrefix(string_view str1, string_view str2)
	{
		const auto mismatchResult = std::mismatch(str1.begin(), str1.end(), str2.begin(), str2.end());
		return str1.substr(0, std::distance(str1.begin(), mismatchResult.first));
	}


	inline string_view ExtractCommonSuffix(string_view str1, string_view str2)
	{
		const auto mismatchResult = std::mismatch(str1.rbegin(), str1.rend(), str2.rbegin(), str2.rend());
		return str1.substr(str1.size() - std::distance(str1.rbegin(), mismatchResult.first));
	}


	inline string_view RemovePrefix(string_view str, string_view prefix)
	{ return str.compare(0, prefix.length(), prefix) == 0 ? str.substr(prefix.length()) : str; }


	inline string_view RemoveSuffix(string_view str, string_view suffix)
	{
		if (str.length() < suffix.length())
			return str;
		return str.compare(str.length() - suffix.length(), suffix.length(), suffix) == 0 ? str.substr(0, str.length() - suffix.length()) : str;
	}


	inline bool StartsWith(string_view str, string_view prefix)
	{ return str.size() >= prefix.size() && str.compare(0, prefix.size(), prefix) == 0; }


	inline bool EndsWith(string_view str, string_view suffix)
	{ return str.size() >= suffix.size() && str.compare(str.size() - suffix.size(), suffix.size(), suffix) == 0; }


	inline bool Contains(string_view str, string_view substr)
	{ return str.find(substr) != string_view::npos; }


	namespace Detail
	{

		struct DelimiterMatch
		{
			size_t				Position;
			size_t				Size;

			DelimiterMatch() : Position(string_view::npos), Size() { }
			DelimiterMatch(size_t pos, size_t size): Position(pos), Size(size) { }
		};

	}


	class IsAnyOf
	{
	private:
		string_view				_list;

	public:
		explicit IsAnyOf(string_view list) : _list(list) { }

		Detail::DelimiterMatch operator () (string_view string, size_t startPos) const
		{ return Detail::DelimiterMatch(string.find_first_of(_list, startPos), 1); }
	};


	namespace Detail
	{

		class StaticSplitDelimiter
		{
		private:
			string_view						_delimiter;

		public:
			explicit StaticSplitDelimiter(string_view delimiter)
				:	_delimiter(delimiter)
			{ STINGRAYKIT_CHECK(!_delimiter.empty(), ArgumentException("delimiter")); }

			Detail::DelimiterMatch operator () (string_view string, size_t startPos) const
			{ return Detail::DelimiterMatch(string.find(_delimiter, startPos), _delimiter.size()); }
		};

		template < typename StringSearchType >
		class SplitStringRange : public Range::RangeBase<SplitStringRange<StringSearchType>, string_view, std::forward_iterator_tag>
		{
			using Self = SplitStringRange;

		public:
			static const size_t				NoLimit = 0;

		private:
			StringSearchType				_search;
			string_view						_string;
			size_t							_startPos;
			DelimiterMatch					_next;
			size_t							_results;
			size_t							_resultsLimit;

		public:
			SplitStringRange(const StringSearchType& search, string_view string, size_t limit)
				: _search(search), _string(string), _startPos(0), _results(0), _resultsLimit(limit)
			{ First(); }

			bool Valid() const
			{ return _startPos != string_view::npos; }

			typename Self::ValueType Get() const
			{
				STINGRAYKIT_CHECK(Valid(), "Get() behind last element");
				return _string.substr(_startPos, _next.Position == string_view::npos ? string_view::npos : _next.Position - _startPos);
			}

			bool Equals(const SplitStringRange& other) const
			{ return _startPos == other._startPos; }

			Self& First()
			{
				_startPos = 0;
				_results = 0;
				_next = _search(_string, _startPos);
				return *this;
			}

			Self& Next()
			{
				STINGRAYKIT_CHECK(Valid(), "Next() behind last element");
				_startPos = _next.Position;
				if (_startPos != string_view::npos)
				{
					_startPos += _next.Size;
					_next = _resultsLimit == NoLimit || ++_results < _resultsLimit ? _search(_string, _startPos) : Detail::DelimiterMatch();
				}
				return *this;
			}

			Self& End()
			{
				_startPos = string_view::npos;
				return *this;
			}
		};

		using StaticDelimiterSplitStringRange = SplitStringRange<StaticSplitDelimiter>;

	}


	inline Detail::StaticDelimiterSplitStringRange Split(string_view string, string_view delimiter, size_t limit = Detail::StaticDelimiterSplitStringRange::NoLimit)
	{ return Detail::StaticDelimiterSplitStringRange(Detail::StaticSplitDelimiter(delimiter), string, limit); }

	inline Detail::StaticDelimiterSplitStringRange Split(string_view string, const std::string& delimiter, size_t limit = Detail::StaticDelimiterSplitStringRange::NoLimit)
	{ return Split(string, string_view(delimiter), limit); }

	inline Detail::StaticDelimiterSplitStringRange Split(string_view string, const char* delimiter, size_t limit = Detail::StaticDelimiterSplitStringRange::NoLimit)
	{ return Split(string, string_view(delimiter), limit); }

	template < typename StringSearchType >
	Detail::SplitStringRange<StringSearchType> Split(string_view string, const StringSearchType& search, size_t limit = Detail::SplitStringRange<StringSearchType>::NoLimit)
	{ return Detail::SplitStringRange<StringSearchType>(search, string, limit); }


	template < typename Range, typename UnaryOperator >
	std::string Join(string_view separator, const Range& range_, const UnaryOperator& op)
	{
		Range range(range_);
		string_ostream result;

		if (range.Valid())
		{
			result << FunctorInvoker::InvokeArgs(op, range.Get());
			range.Next();
		}

		for (; range.Valid(); range.Next())
			result << separator << FunctorInvoker::InvokeArgs(op, range.Get());

		return result.str();
	}


	template < typename Range >
	inline std::string Join(string_view separator, const Range& range)
	{ return Join(separator, range, ToStringPrinter()); }


	template < typename InputIterator, typename UnaryOperator >
	inline std::string Join(string_view separator, const InputIterator& first, const InputIterator& last, const UnaryOperator& op)
	{
		string_ostream result;

		for (auto it = first; it != last; ++it)
		{
			if (it != first)
				result << separator;
			result << FunctorInvoker::InvokeArgs(op, *it);
		}

		return result.str();
	}


	template < typename InputIterator >
	inline std::string Join(string_view separator, const InputIterator& first, const InputIterator& last)
	{ return Join(separator, first, last, ToStringPrinter()); }


	inline string_view RightStrip(string_view str, string_view chars = " \t\n\r\f\v")
	{
		const size_t pos = str.find_last_not_of(chars);
		return pos == string_view::npos ? string_view() : str.substr(0, pos + 1);
	}


	inline string_view LeftStrip(string_view str, string_view chars = " \t\n\r\f\v")
	{
		const size_t pos = str.find_first_not_of(chars);
		return pos == string_view::npos ? string_view() : str.substr(pos);
	}


	inline string_view Strip(string_view str, string_view chars = " \t\n\r\f\v")
	{ return LeftStrip(RightStrip(str, chars), chars); }


	template < typename Transformer >
	std::string Transform(string_view str, const Transformer& transformer)
	{
		string_ostream result;
		std::transform(str.begin(), str.end(), std::back_inserter(result), transformer);
		return result.str();
	}


	inline std::string ToLower(string_view str)
	{ return Transform(str, &::tolower); }


	inline std::string ToUpper(string_view str)
	{ return Transform(str, &::toupper); }


	inline std::string Capitalize(string_view str)
	{ return str.empty() ? std::string() : str.copy().replace(0, 1, 1, ::toupper(str[0])); }


	inline std::string Uncapitalize(string_view str)
	{ return str.empty() ? std::string() : str.copy().replace(0, 1, 1, ::tolower(str[0])); }


	namespace Detail
	{

		template < bool Left >
		struct StringJustificator
		{
			string_view			String;
			size_t				Width;
			char				Filler;

			StringJustificator(string_view str, size_t width, char ch) : String(str), Width(width), Filler(ch) { }

			operator std::string () const
			{
				return Left ?
						StringBuilder() % String % std::string(String.size() < Width ? Width - String.size() : 0, Filler) :
						StringBuilder() % std::string(String.size() < Width ? Width - String.size() : 0, Filler) % String;
			}
		};

		template < typename StringType, bool Left >
		string_ostream& operator << (string_ostream& stream, const StringJustificator<Left>& sj)
		{
			if (Left)
				stream << sj.String;
			if (sj.String.size() < sj.Width)
			{
				size_t width = sj.Width - sj.String.size();
				while (width--)
					stream << sj.Filler;
			}
			if (!Left)
				stream << sj.String;
			return stream;
		}

	}


	inline Detail::StringJustificator<true> LeftJustify(string_view str, size_t width, char filler = ' ')
	{ return Detail::StringJustificator<true>(str, width, filler); }


	inline Detail::StringJustificator<false> RightJustify(string_view str, size_t width, char filler = ' ')
	{ return Detail::StringJustificator<false>(str, width, filler); }


	namespace Detail
	{

		template < typename TupleType >
		struct TupleFromStringsHelper
		{
			template < size_t Index >
			struct Functor
			{
				using Type = typename GetTypeListItem<typename TupleType::Types, Index>::ValueT;

				template < typename Range_ >
				static bool Call(const TupleType& tuple, Range_* range)
				{
					if (!range->Valid())
						return false;

					tuple.template Get<Index>() = lexical_cast<typename RemoveReference<Type>::ValueT>(range->Get());
					range->Next();
					return true;
				}
			};
		};

	}


	template < typename TupleType, typename SourceRange_ >
	void TupleFromStrings(const TupleType& tuple, const SourceRange_& inputRange_)
	{
		SourceRange_ inputRange(inputRange_);
		const bool result = ForIf<TupleType::Size, Detail::TupleFromStringsHelper<TupleType>::template Functor>::Do(tuple, &inputRange);
		STINGRAYKIT_CHECK(result && !inputRange.Valid(), "Inappropriate range size");
	}


	template < typename TupleType, typename SourceRange_ >
	bool TryTupleFromStrings(const TupleType& tuple, const SourceRange_& inputRange_)
	{
		SourceRange_ inputRange(inputRange_);

		bool result = false;
		try
		{ result = ForIf<TupleType::Size, Detail::TupleFromStringsHelper<TupleType>::template Functor>::Do(tuple, &inputRange); }
		catch (const std::exception&)
		{ }

		return result && !inputRange.Valid();
	}


	string_view::size_type EditDistance(string_view s1, string_view s2);

}

#endif
