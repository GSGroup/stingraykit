#ifndef STINGRAYKIT_STRING_STRINGUTILS_H
#define STINGRAYKIT_STRING_STRINGUTILS_H

// Copyright (c) 2011 - 2017, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/Dummy.h>
#include <stingraykit/Macro.h>
#include <stingraykit/Types.h>
#include <stingraykit/collection/CollectionBuilder.h>
#include <stingraykit/collection/IEnumerable.h>
#include <stingraykit/collection/Range.h>
#include <stingraykit/exception.h>
#include <stingraykit/metaprogramming/NestedTypeCheck.h>
#include <stingraykit/optional.h>
#include <stingraykit/shared_ptr.h>
#include <stingraykit/string/lexical_cast.h>

#include <algorithm>
#include <ctype.h>
#include <string>

namespace std
{
	template < class Key, class T, class Compare, class Allocator > class map;
}


namespace stingray
{

	template < typename CharType >
	std::basic_string<CharType>& ReplaceAll(std::basic_string<CharType>& str,
					const std::basic_string<CharType>& replaceSeq,
					const std::basic_string<CharType>& replaceTo,
					Dummy dummy = Dummy())
	{
		typedef std::basic_string<CharType>	StrType;

		typename StrType::size_type i = str.find(replaceSeq);
		while (i != StrType::npos)
		{
			str.replace(i, replaceSeq.size(), replaceTo);
			i = str.find(replaceSeq, i + replaceTo.size());
		}
		return str;
	}


	inline std::string& ReplaceAll(std::string& str, const std::string& replaceSeq, const std::string& replaceTo)
	{ return ReplaceAll<char>(str, replaceSeq, replaceTo); }


	inline std::string& ReplaceAll(std::string& str, char from, char to)
	{
		for(std::string::iterator it = str.begin(); it != str.end(); ++it)
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
		return str.compare(str.length() - suffix.length(), suffix.length(), suffix) == 0? str.substr(0, str.length() - suffix.length()) : str;
	}


	inline bool BeginsWith(const std::string& str, const std::string& prefix)
	{ return str.size() >= prefix.size() && str.compare(0, prefix.size(), prefix) == 0; }


	inline bool EndsWith(const std::string& str, const std::string& suffix)
	{ return str.size() >= suffix.size() && str.compare(str.size() - suffix.size(), suffix.size(), suffix) == 0; }


	inline bool Contains(const std::string& str, const std::string& substr)
	{ return str.find(substr) != std::string::npos; }


	class StringRef
	{
	public:
		typedef std::string::size_type	size_type;

		static const size_type npos = std::string::npos;

	private:
		const std::string *		_owner;
		std::string::size_type	_begin;
		std::string::size_type	_end;

	public:
		inline StringRef(const std::string& owner, size_t begin = 0, size_t end = npos) : _owner(&owner), _begin(begin), _end(end != npos ? end : owner.size()) { }

		inline bool empty() const		{ return _begin >= _end; }
		inline size_type size() const	{ return _end >= _begin ? _end - _begin : 0; }

		inline size_type find(const char c, size_type pos = 0) const
		{
			size_type p = _owner->find(c, pos + _begin);
			return (p >= _end) ? npos : p - _begin;
		}

		inline size_type find(const std::string& str, size_type pos = 0) const
		{
			size_type p = _owner->find(str, pos + _begin);
			return (p >= _end) ? npos : p - _begin;
		}

		StringRef substr(size_type pos = 0, size_type n = npos) const
		{ return StringRef(*_owner, _begin + pos, n == npos? _end: _begin + pos + n ); }

		char operator[] (size_type index) const
		{ return (*_owner)[_begin + index]; }

		inline std::string str() const { return str_substr(); }
		operator std::string() const   { return str(); }

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
		const std::string & _list;

	public:
		IsAnyOf(const std::string & list): _list(list)
		{ }

		template<typename StringLikeObject>
		Detail::DelimiterMatch operator()(const StringLikeObject & string, size_t startPos)
		{ return Detail::DelimiterMatch(string.find_first_of(_list, startPos), 1); }
	};

	namespace Detail
	{
		class StaticSplitDelimiter
		{
			std::string		_delimiter;

		public:
			StaticSplitDelimiter(const std::string &delimiter): _delimiter(delimiter) { }

			template<typename StringLikeObject>
			Detail::DelimiterMatch operator()(const StringLikeObject &string, size_t startPos) const
			{ return Detail::DelimiterMatch(string.find(_delimiter, startPos), _delimiter.size()); }

			size_t size() const
			{ return _delimiter.size(); }
		};

		class SplitStringValueProxy
		{
			StringRef						_ref;

		public:
			SplitStringValueProxy(const StringRef &ref) : _ref(ref) { }

			std::string ToString() const
			{ return _ref.str(); }

			operator std::string () const
			{ return _ref.str(); }

			operator const StringRef & () const
			{ return _ref; }

/* fixme: make me work
			template<typename R>
			operator typename EnableIf
			<
				IsIntType<typename Deconst<typename Dereference<R>::ValueT>::ValueT>::Value,
				typename Deconst<typename Dereference<R>::ValueT>::ValueT
			>::ValueT () const
			{
				typedef typename Deconst<typename Dereference<R>::ValueT>::ValueT ValueType;
				return FromString<ValueType>(_ref.str());
			}
*/
		};


		template<typename StringSearchType>
		class SplitStringRange :
			public Range::RangeBase<SplitStringRange<StringSearchType>, StringRef, std::forward_iterator_tag>
		{
			typedef SplitStringRange<StringSearchType> Self;
			typedef Range::RangeBase<SplitStringRange<StringSearchType>, StringRef, std::forward_iterator_tag> base;

		public:
			typedef std::string				ValueType;
			static const size_t				NoLimit = 0;

		private:
			StringSearchType				_search;
			const ValueType &				_string;
			size_t							_startPos;
			DelimiterMatch					_next;
			size_t							_results;
			size_t							_resultsLimit;

		public:
			SplitStringRange(const StringSearchType &search, const ValueType &string, size_t limit) :
				_search(search), _string(string), _startPos(0), _results(0), _resultsLimit(limit)
			{ _next = _search(_string, _startPos); }

			bool Valid() const
			{ return _startPos != std::string::npos; }

			typename base::ValueType Get()
			{ return StringRef(_string, _startPos, _next.Position); }

			bool Equal(const SplitStringRange& other) const
			{ return _startPos == other._startPos; }

			Self& First()
			{ _startPos = 0; _results = 0; return *this; }

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


		typedef SplitStringRange<StaticSplitDelimiter>	StaticDelimiterSplitStringRange;
		typedef SplitStringRange<IsAnyOf>				IsAnyOfSplitStringRange;
	}


	inline Detail::StaticDelimiterSplitStringRange Split(const std::string &string, const std::string &delimiter, size_t limit = Detail::StaticDelimiterSplitStringRange::NoLimit)
	{ return Detail::StaticDelimiterSplitStringRange(Detail::StaticSplitDelimiter(delimiter), string, limit); }


	inline Detail::IsAnyOfSplitStringRange Split(const std::string &string, const IsAnyOf &search, size_t limit = Detail::IsAnyOfSplitStringRange::NoLimit)
	{ return Detail::IsAnyOfSplitStringRange(search, string, limit); }


	template < typename Range, typename UnaryOperator >
	std::string Join(const std::string& separator, Range range, UnaryOperator op)
	{
		std::string result;
		for (; range.Valid(); range.Next())
		{
			if (!result.empty())
				result.append(separator);

			result.append(op(range.Get()));
		}
		return result;
	}


	template < typename Range >
	inline std::string Join(const std::string& separator, Range range)
	{ return Join(separator, range, lexical_cast<std::string, typename Range::ValueType>); }


	template < typename InputIterator, typename UnaryOperator >
	inline std::string Join(const std::string& separator, InputIterator first, InputIterator last, UnaryOperator op)
	{ return Join(separator, ToRange(first, last), op); }


	template < typename InputIterator >
	inline std::string Join(const std::string& separator, InputIterator first, InputIterator last)
	{ return Join(separator, ToRange(first, last), lexical_cast<std::string, typename std::iterator_traits<InputIterator>::value_type>); }


	inline std::string RightStrip(const std::string& str, const std::string& chars = " \t\n\r\f\v")
	{
		const size_t pos = str.find_last_not_of(chars);
		return pos == std::string::npos? "" : str.substr(0, pos + 1);
	}


	inline std::string LeftStrip(const std::string& str, const std::string& chars = " \t\n\r\f\v")
	{
		const size_t pos = str.find_first_not_of(chars);
		return pos == std::string::npos? "" : str.substr(pos);
	}


	inline std::string Strip(const std::string& str, const std::string& chars = " \t\n\r\f\v")
	{ return LeftStrip(RightStrip(str, chars), chars); }


	template< typename Transformer >
	std::string Transform(const std::string& str, Transformer transformer)
	{
		string_ostream result;
		std::transform(str.begin(), str.end(), std::back_inserter(result), transformer);
		return result.str();
	}


	inline std::string ToLower(const std::string& str)
	{ return Transform(str, ::tolower); }


	inline std::string ToUpper(const std::string& str)
	{ return Transform(str, ::toupper); }


	inline std::string Capitalize(const std::string& str)
	{ return str.empty()? str : ToUpper(str.substr(0, 1)) + str.substr(1); }


	inline std::string Uncapitalize(const std::string& str)
	{ return str.empty()? str : ToLower(str.substr(0, 1)) + str.substr(1); }

	namespace Detail
	{

		template<typename StringType, bool Left>
		struct StringJustificator
		{
			const StringType &	String;
			size_t				Width;
			char				Filler;

			StringJustificator(const StringType & str, size_t width, char ch): String(str), Width(width), Filler(ch) { }

			operator std::string () const
			{
				return Left?
					(std::string)String + std::string(String.size() < Width? Width - String.size() : 0, Filler):
					std::string(String.size() < Width? Width - String.size() : 0, Filler) + (std::string)String;
			}
		};

		template<typename StreamType, typename StringType, bool Left>
		StreamType & operator<< (StreamType & stream, const StringJustificator<StringType, Left> & rj)
		{
			if (Left)
				stream << rj.String;
			if (rj.String.size() < rj.Width)
			{
				size_t width = rj.Width - rj.String.size();
				while(width--)
					stream << rj.Filler;
			}
			if (!Left)
				stream << rj.String;
			return stream;
		}
	}

	inline Detail::StringJustificator<std::string, true> LeftJustify(const std::string& str, size_t width, char filler = ' ')
	{ return Detail::StringJustificator<std::string, true>(str, width, filler); }

	inline Detail::StringJustificator<std::string, false> RightJustify(const std::string& str, size_t width, char filler = ' ')
	{ return Detail::StringJustificator<std::string, false>(str, width, filler); }


	namespace Detail
	{
		template<typename TupleType>
		struct TupleFromStringsHelper
		{
			template<int Index>
			struct Functor
			{
				typedef typename GetTypeListItem<typename TupleType::TypeList, Index>::ValueT Type;

				template<typename Range_>
				static void Call(const TupleType& tuple, Range_* range)
				{
					STINGRAYKIT_CHECK(range->Valid(), "not enough data to fill output range");
					Type ptr = tuple.template Get<Index>();
					*ptr = lexical_cast<typename Depointer<Type>::ValueT>(range->Get());
					range->Next();
				}
			};
		};
	}


	template<typename TupleType, typename SourceRange_>
	SourceRange_ TupleFromStrings(const TupleType& tuple, SourceRange_ inputRange)
	{
		stingray::For<TupleType::Size, Detail::TupleFromStringsHelper<TupleType>::template Functor>::Do(tuple, &inputRange);
		return inputRange;
	}

}


#endif
