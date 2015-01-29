#ifndef STINGRAYKIT_STRING_STRINGUTILS_H
#define STINGRAYKIT_STRING_STRINGUTILS_H


#include <algorithm>
#include <ctype.h>
#include <string>

#include <stingraykit/Dummy.h>
#include <stingraykit/Macro.h>
#include <stingraykit/NestedTypeCheck.h>
#include <stingraykit/Types.h>
#include <stingraykit/collection/CollectionBuilder.h>
#include <stingraykit/collection/IEnumerable.h>
#include <stingraykit/exception.h>
#include <stingraykit/optional.h>
#include <stingraykit/shared_ptr.h>
#include <stingraykit/string/ToString.h>
#include <stingraykit/string/string_stream.h>


namespace std
{
	template < class Key, class T, class Compare, class Allocator > class map;
}


namespace stingray
{

	namespace Detail
	{

		template< typename To, typename From >
		struct LexicalCast;


		template< typename To >
		struct LexicalCast<To, std::string>
		{
			static To Do(const std::string& from)
			{ return FromString<To>(from); }
		};


		template< typename From >
		struct LexicalCast<std::string, From>
		{
			static std::string Do(const From& from)
			{ return ToString(from); }
		};


		template < >
		struct LexicalCast<std::string, std::string>
		{
			static std::string Do(const std::string& from)
			{ return from; }
		};

	}


	template < typename To, typename From >
	To lexical_cast(const From& from)
	{ return Detail::LexicalCast<To, From>::Do(from); }


	namespace Detail
	{

		template < typename From >
		class LexicalCasterProxy
		{
		private:
			From	_from;

		public:
			explicit LexicalCasterProxy(const From& from)
				: _from(from)
			{ }

			template < typename To >
			operator To() const
			{ return lexical_cast<To>(_from); }
		};

	}


	template < typename From >
	Detail::LexicalCasterProxy<From> lexical_caster(const From& from)
	{ return Detail::LexicalCasterProxy<From>(from); }


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
	{ return str.length() >= prefix.length() && ExtractPrefix(str, prefix.length()) == prefix; }


	inline bool EndsWith(const std::string& str, const std::string& suffix)
	{ return str.length() >= suffix.length() && ExtractSuffix(str, suffix.length()) == suffix; }


	namespace Detail
	{

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

			inline std::string substr(size_type pos = 0, size_type n = npos) const
			{
				return pos < size() ? _owner->substr(_begin + pos, std::min(size() - pos, n)) : std::string();
			}

			inline std::string str() const { return substr(); }
		};


		template<typename ContainerType>
		inline void SplitRefsImpl(const std::string& sourceString, const std::vector<std::string>& delimiters, ContainerType& result, int maxsplit)
		{
			size_t lastPosition = 0;
			size_t delimiterPosition;
			int counter = 0;
			do
			{
				delimiterPosition = std::string::npos;
				size_t delimiterSize = 0;

				for (size_t i = 0; i < delimiters.size(); ++i)
				{
					size_t position;
					if ((position = sourceString.find(delimiters[i], lastPosition)) != std::string::npos && (delimiterPosition == std::string::npos || position < delimiterPosition))
					{
						delimiterPosition = position;
						delimiterSize = delimiters[i].length();
					}
				}

				if (delimiterPosition != std::string::npos && (maxsplit < 0 || counter < maxsplit))
				{
					result.push_back(StringRef(sourceString, lastPosition, delimiterPosition));
					lastPosition = delimiterPosition + delimiterSize;
					++counter;
				}
			}
			while (delimiterPosition != std::string::npos && (maxsplit < 0 || counter < maxsplit));

			result.push_back(StringRef(sourceString, lastPosition));
		}


		template<typename ContainerType, typename UnaryOperator, typename ValueType>
		struct SplitImpl
		{
			void operator()(const std::string& sourceString, const std::vector<std::string>& delimiters, ContainerType& result, UnaryOperator op, int maxsplit)
			{
				std::vector<StringRef> refs;
				SplitRefsImpl(sourceString, delimiters, refs, maxsplit);

				for (size_t i = 0; i < refs.size(); ++i)
					Detail::CollectionInserter<ContainerType>::Insert(result, op(refs[i].str()));
			}
		};

	}


	typedef Detail::StringRef	StringRef;


#define DETAIL_SPLIT_PARAM_USAGE(Index_, UserArg_) % STINGRAYKIT_CAT(p, Index_)

#define DETAIL_STINGRAYKIT_DECLARE_SPLIT_FUNCTION(ParamsCount_, UserData_) \
	STINGRAYKIT_INSERT_IF(ParamsCount_, \
		template <typename ContainerType> \
		inline void Split(const std::string& str, STINGRAYKIT_REPEAT(ParamsCount_, STINGRAYKIT_FUNCTION_TYPED_PARAM_DECL, std::string), ContainerType& result, int maxsplit = -1, Detail::SplitImpl<ContainerType, typename ContainerType::value_type(*)(const std::string&), typename ContainerType::value_type> splitImpl = Detail::SplitImpl<ContainerType, typename ContainerType::value_type(*)(const std::string&), typename ContainerType::value_type>()) \
		{ \
			splitImpl(str, VectorBuilder<std::string>() STINGRAYKIT_REPEAT(ParamsCount_, DETAIL_SPLIT_PARAM_USAGE, STINGRAYKIT_EMPTY()), result, lexical_cast<typename ContainerType::value_type, std::string>, maxsplit); \
		} \
	) \
	STINGRAYKIT_INSERT_IF(ParamsCount_, \
		template <typename ContainerType STINGRAYKIT_COMMA typename UnaryOperator> \
		inline void Split(const std::string& str, STINGRAYKIT_REPEAT(ParamsCount_, STINGRAYKIT_FUNCTION_TYPED_PARAM_DECL, std::string), ContainerType& result, UnaryOperator op, int maxsplit = -1, Detail::SplitImpl<ContainerType, UnaryOperator, typename ContainerType::value_type> splitImpl = Detail::SplitImpl<ContainerType, UnaryOperator, typename ContainerType::value_type>()) \
		{ \
			splitImpl(str, VectorBuilder<std::string>() STINGRAYKIT_REPEAT(ParamsCount_, DETAIL_SPLIT_PARAM_USAGE, STINGRAYKIT_EMPTY()), result, op, maxsplit); \
		} \
	)

	STINGRAYKIT_REPEAT_NESTING_2(10, DETAIL_STINGRAYKIT_DECLARE_SPLIT_FUNCTION, STINGRAYKIT_EMPTY())

#define DETAIL_STINGRAYKIT_DECLARE_SPLITREFS_FUNCTION(ParamsCount_, UserData_) \
	STINGRAYKIT_INSERT_IF(ParamsCount_, \
		template <typename ContainerType> \
		inline void SplitRefs(const std::string& str, STINGRAYKIT_REPEAT(ParamsCount_, STINGRAYKIT_FUNCTION_TYPED_PARAM_DECL, std::string), ContainerType& result, int maxsplit = -1) \
		{ \
			Detail::SplitRefsImpl(str, VectorBuilder<std::string>() STINGRAYKIT_REPEAT(ParamsCount_, DETAIL_SPLIT_PARAM_USAGE, STINGRAYKIT_EMPTY()), result, maxsplit); \
		} \
	)

	STINGRAYKIT_REPEAT_NESTING_2(10, DETAIL_STINGRAYKIT_DECLARE_SPLITREFS_FUNCTION, STINGRAYKIT_EMPTY())

#undef DETAIL_SPLIT_PARAM_USAGE
#undef DETAIL_STINGRAYKIT_DECLARE_SPLIT_FUNCTION
#undef DETAIL_STINGRAYKIT_DECLARE_SPLITREFS_FUNCTION


	template < typename InputIterator, typename UnaryOperator >
	std::string Join(const std::string& separator, InputIterator first, InputIterator last, UnaryOperator op)
	{
		std::string result;
		while (first != last)
		{
			if (!result.empty())
				result.append(separator);

			result.append(op(*first));
			++first;
		}
		return result;
	}


	template < typename InputIterator >
	std::string Join(const std::string& separator, InputIterator first, InputIterator last)
	{ return Join(separator, first, last, lexical_cast<std::string, typename std::iterator_traits<InputIterator>::value_type>); }


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


	inline std::string LeftJustify(const std::string& str, size_t width, char filler = ' ')
	{ return str + std::string(str.length() < width? width - str.length() : 0, filler); }


	inline std::string RightJustify(const std::string& str, size_t width, char filler = ' ')
	{ return std::string(str.length() < width? width - str.length() : 0, filler) + str;  }

}


#endif
