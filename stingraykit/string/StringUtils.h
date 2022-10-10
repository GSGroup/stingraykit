#ifndef STINGRAYKIT_STRING_STRINGUTILS_H
#define STINGRAYKIT_STRING_STRINGUTILS_H

// Copyright (c) 2011 - 2022, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/collection/Range.h>
#include <stingraykit/string/lexical_cast.h>

#include <algorithm>
#include <ctype.h>

namespace stingray
{

	template < typename CharType >
	std::basic_string<CharType>& ReplaceAll(std::basic_string<CharType>& str,
			const std::basic_string<CharType>& replaceSeq,
			const std::basic_string<CharType>& replaceTo,
			Dummy dummy = Dummy())
	{
		using StringType = std::basic_string<CharType>;

		typename StringType::size_type index = str.find(replaceSeq);
		while (index != StringType::npos)
		{
			str.replace(index, replaceSeq.size(), replaceTo);
			index = str.find(replaceSeq, index + replaceTo.size());
		}
		return str;
	}


	inline std::string& ReplaceAll(std::string& str, const std::string& replaceSeq, const std::string& replaceTo)
	{ return ReplaceAll<char>(str, replaceSeq, replaceTo); }


	inline std::string& ReplaceAll(std::string& str, char from, char to)
	{
		for (std::string::iterator it = str.begin(); it != str.end(); ++it)
			if (*it == from)
				*it = to;
		return str;
	}


	inline std::string Filter(const std::string& str, const std::string& characters)
	{
		std::string result;
		for (std::string::const_iterator it = str.begin(); it != str.end(); ++it)
			if (characters.find(*it) == std::string::npos)
				result.push_back(*it);
		return result;
	}


	inline std::string Remove(const std::string& str, char ch = ' ')
	{
		std::string result(str);
		result.erase(std::remove(result.begin(), result.end(), ch), result.end());
		return result;
	}


	inline std::string ExtractPrefix(const std::string& str, size_t prefixLength)
	{ return str.substr(0, std::min(str.length(), prefixLength)); }


	inline std::string ExtractSuffix(const std::string& str, size_t suffixLength)
	{
		const size_t length = std::min(str.length(), suffixLength);
		return str.substr(str.length() - length, length);
	}


	inline std::string RemovePrefix(const std::string& str, const std::string& prefix)
	{ return str.compare(0, prefix.length(), prefix) == 0? str.substr(prefix.length()) : str; }


	inline std::string RemoveSuffix(const std::string& str, const std::string& suffix)
	{
		if (str.length() < suffix.length())
			return str;
		return str.compare(str.length() - suffix.length(), suffix.length(), suffix) == 0 ? str.substr(0, str.length() - suffix.length()) : str;
	}


	inline bool StartsWith(const std::string& str, const std::string& prefix)
	{ return str.size() >= prefix.size() && str.compare(0, prefix.size(), prefix) == 0; }


	inline bool EndsWith(const std::string& str, const std::string& suffix)
	{ return str.size() >= suffix.size() && str.compare(str.size() - suffix.size(), suffix.size(), suffix) == 0; }


	inline bool Contains(const std::string& str, const std::string& substr)
	{ return str.find(substr) != std::string::npos; }


	class StringRef
	{
	public:
		using  size_type = std::string::size_type;

		static const size_type npos = std::string::npos;

	private:
		const std::string*		_owner;
		std::string::size_type	_begin;
		std::string::size_type	_end;

	public:
		StringRef(const std::string& owner, size_t begin = 0, size_t end = npos) : _owner(&owner), _begin(begin), _end(end != npos ? end : owner.size()) { }

		bool empty() const		{ return _begin >= _end; }
		size_type size() const	{ return _end >= _begin ? _end - _begin : 0; }

		size_type find(const char c, size_type pos = 0) const
		{
			const size_type p = _owner->find(c, pos + _begin);
			return p >= _end ? npos : p - _begin;
		}

		size_type find(const std::string& str, size_type pos = 0) const
		{
			const size_type p = _owner->find(str, pos + _begin);
			return p >= _end ? npos : p - _begin;
		}

		StringRef substr(size_type pos = 0, size_type n = npos) const
		{ return StringRef(*_owner, _begin + pos, n == npos ? _end : _begin + pos + n ); }

		char operator [] (size_type index) const
		{ return (*_owner)[_begin + index]; }

		std::string str() const	{ return str_substr(); }
		operator std::string () const	{ return str(); }

		bool operator != (const std::string& other) const { return str() != other; }
		bool operator == (const std::string& other) const { return str() == other; }

	private:
		std::string str_substr(size_type pos = 0, size_type n = npos) const
		{ return pos < size() ? _owner->substr(_begin + pos, std::min(size() - pos, n)) : std::string(); }
	};


	namespace Detail
	{

		struct DelimiterMatch
		{
			size_t Position;
			size_t Size;

			DelimiterMatch() : Position(std::string::npos), Size() { }
			DelimiterMatch(size_t pos, size_t size): Position(pos), Size(size) { }
		};

	}


	class IsAnyOf
	{
	private:
		const std::string&		_list;

	public:
		IsAnyOf(const std::string& list) : _list(list)
		{ }

		template < typename StringLikeObject >
		Detail::DelimiterMatch operator () (const StringLikeObject& string, size_t startPos)
		{ return Detail::DelimiterMatch(string.find_first_of(_list, startPos), 1); }
	};


	namespace Detail
	{
		class StaticSplitDelimiter
		{
		private:
			std::string		_delimiter;

		public:
			StaticSplitDelimiter(const std::string& delimiter) : _delimiter(delimiter) { }

			template < typename StringLikeObject >
			Detail::DelimiterMatch operator () (const StringLikeObject& string, size_t startPos) const
			{ return Detail::DelimiterMatch(string.find(_delimiter, startPos), _delimiter.size()); }

			size_t size() const
			{ return _delimiter.size(); }
		};

		template < typename StringSearchType >
		class SplitStringRange
			:	public Range::RangeBase<SplitStringRange<StringSearchType>, StringRef, std::forward_iterator_tag>
		{
			using Self = SplitStringRange<StringSearchType>;
			using base = Range::RangeBase<SplitStringRange<StringSearchType>, StringRef, std::forward_iterator_tag>;

		public:
			using ValueType = std::string;

			static const size_t				NoLimit = 0;

		private:
			StringSearchType				_search;
			const ValueType&				_string;
			size_t							_startPos;
			DelimiterMatch					_next;
			size_t							_results;
			size_t							_resultsLimit;

		public:
			SplitStringRange(const StringSearchType& search, const ValueType& string, size_t limit)
				: _search(search), _string(string), _startPos(0), _results(0), _resultsLimit(limit)
			{ _next = _search(_string, _startPos); }

			bool Valid() const
			{ return _startPos != std::string::npos; }

			typename base::ValueType Get() const
			{
				STINGRAYKIT_CHECK(Valid(), "Get() behind last element");
				return StringRef(_string, _startPos, _next.Position);
			}

			bool Equals(const SplitStringRange& other) const
			{ return _startPos == other._startPos; }

			Self& First()
			{
				_startPos = 0;
				_results = 0;
				return *this;
			}

			Self& Next()
			{
				STINGRAYKIT_CHECK(Valid(), "Invalid range advanced!");
				_startPos = _next.Position;
				if (_startPos != std::string::npos)
				{
					_startPos += _next.Size;
					_next = (_resultsLimit == NoLimit || ++_results < _resultsLimit) ? _search(_string, _startPos) : Detail::DelimiterMatch();
				}
				return *this;
			}
		};


		using StaticDelimiterSplitStringRange = SplitStringRange<StaticSplitDelimiter>;
		using IsAnyOfSplitStringRange = SplitStringRange<IsAnyOf>;
	}


	inline Detail::StaticDelimiterSplitStringRange Split(const std::string& string, const std::string& delimiter, size_t limit = Detail::StaticDelimiterSplitStringRange::NoLimit)
	{ return Detail::StaticDelimiterSplitStringRange(Detail::StaticSplitDelimiter(delimiter), string, limit); }


	inline Detail::IsAnyOfSplitStringRange Split(const std::string& string, const IsAnyOf& search, size_t limit = Detail::IsAnyOfSplitStringRange::NoLimit)
	{ return Detail::IsAnyOfSplitStringRange(search, string, limit); }


	template < typename Range, typename UnaryOperator >
	std::string Join(const std::string& separator, const Range& range_, const UnaryOperator& op)
	{
		Range range(range_);
		StringJoiner sj(separator);
		for (; range.Valid(); range.Next())
			sj % FunctorInvoker::InvokeArgs(op, range.Get());
		return sj;
	}


	template < typename Range >
	inline std::string Join(const std::string& separator, const Range& range)
	{ return Join(separator, range, &lexical_cast<std::string, typename Range::ValueType>); }


	template < typename InputIterator, typename UnaryOperator >
	inline std::string Join(const std::string& separator, const InputIterator& first, const InputIterator& last, const UnaryOperator& op)
	{ return Join(separator, ToRange(first, last), op); }


	template < typename InputIterator >
	inline std::string Join(const std::string& separator, const InputIterator& first, const InputIterator& last)
	{ return Join(separator, ToRange(first, last), &lexical_cast<std::string, typename std::iterator_traits<InputIterator>::value_type>); }


	inline std::string RightStrip(const std::string& str, const std::string& chars = " \t\n\r\f\v")
	{
		const size_t pos = str.find_last_not_of(chars);
		return pos == std::string::npos ? "" : str.substr(0, pos + 1);
	}


	inline std::string LeftStrip(const std::string& str, const std::string& chars = " \t\n\r\f\v")
	{
		const size_t pos = str.find_first_not_of(chars);
		return pos == std::string::npos ? "" : str.substr(pos);
	}


	inline std::string Strip(const std::string& str, const std::string& chars = " \t\n\r\f\v")
	{ return LeftStrip(RightStrip(str, chars), chars); }


	template < typename Transformer >
	std::string Transform(const std::string& str, const Transformer& transformer)
	{
		string_ostream result;
		std::transform(str.begin(), str.end(), std::back_inserter(result), transformer);
		return result.str();
	}


	inline std::string ToLower(const std::string& str)
	{ return Transform(str, &::tolower); }


	inline std::string ToUpper(const std::string& str)
	{ return Transform(str, &::toupper); }


	inline std::string Capitalize(const std::string& str)
	{ return str.empty() ? str : std::string(str).replace(0, 1, 1, ::toupper(str[0])); }


	inline std::string Uncapitalize(const std::string& str)
	{ return str.empty() ? str : std::string(str).replace(0, 1, 1, ::tolower(str[0])); }


	namespace Detail
	{

		template < typename StringType, bool Left >
		struct StringJustificator
		{
			const StringType&	String;
			size_t				Width;
			char				Filler;

			StringJustificator(const StringType& str, size_t width, char ch) : String(str), Width(width), Filler(ch) { }

			operator std::string () const
			{
				return Left ?
						StringBuilder() % String % std::string(String.size() < Width ? Width - String.size() : 0, Filler) :
						StringBuilder() % std::string(String.size() < Width ? Width - String.size() : 0, Filler) % String;
			}
		};

		template < typename StringType, bool Left >
		string_ostream& operator << (string_ostream& stream, const StringJustificator<StringType, Left>& sj)
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

	inline Detail::StringJustificator<std::string, true> LeftJustify(const std::string& str, size_t width, char filler = ' ')
	{ return Detail::StringJustificator<std::string, true>(str, width, filler); }

	inline Detail::StringJustificator<std::string, false> RightJustify(const std::string& str, size_t width, char filler = ' ')
	{ return Detail::StringJustificator<std::string, false>(str, width, filler); }


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
				static void Call(const TupleType& tuple, Range_* range)
				{
					STINGRAYKIT_CHECK(range->Valid(), "not enough data to fill output range");
					tuple.template Get<Index>() = lexical_cast<typename RemoveReference<Type>::ValueT>(range->Get());
					range->Next();
				}
			};
		};
	}


	template < typename TupleType, typename SourceRange_ >
	SourceRange_ TupleFromStrings(const TupleType& tuple, const SourceRange_& inputRange_)
	{
		SourceRange_ inputRange(inputRange_);
		stingray::For<TupleType::Size, Detail::TupleFromStringsHelper<TupleType>::template Functor>::Do(tuple, &inputRange);
		STINGRAYKIT_CHECK(!inputRange.Valid(), "incompletely parsed input range");
		return inputRange;
	}


	std::string::size_type EditDistance(const std::string& s1, const std::string& s2);

}

#endif
