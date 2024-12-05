#ifndef STINGRAYKIT_STRING_STRINGPARSE_H
#define STINGRAYKIT_STRING_STRINGPARSE_H

// Copyright (c) 2011 - 2024, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/variant.h>

#include <deque>
#include <limits>
#include <sstream>

namespace stingray
{

	namespace Detail
	{

		template < typename T, typename ConvertFunc >
		class ParseProxy
		{
		private:
			T&				_value;
			ConvertFunc		_convert;

		public:
			ParseProxy(T& value, const ConvertFunc& convert) : _value(value), _convert(convert) { }

			ParseProxy& operator * () { return *this; }

			void Parse(const std::string& str)
			{ _value = _convert(str); }
		};


		struct FromStringReader
		{
			template < typename T >
			static auto Do(const std::string& str, T& value, int)
					-> decltype(value = FromString<T>(str), bool())
			{
				try
				{ value = FromString<T>(str); }
				catch (const std::exception&)
				{ return false; }

				return true;
			}

			static bool Do(const std::string& string, char& value, int)
			{
				if (string.length() != 1)
					return false;

				value = string[0];
				return true;
			}

			template < typename T >
			static bool Do(const std::string& str, T& value, long)
			{
				std::istringstream stream(str);
				stream >> value;
				return stream.eof();
			}

			template < typename T, typename ConvertFunc >
			static bool Do(const std::string& str, ParseProxy<T, ConvertFunc>& adapter, long)
			{
				try
				{ adapter.Parse(str); }
				catch (const std::exception& ex)
				{ return false; }

				return true;
			}

			template < typename T >
			static bool Do(const std::string& string, optional<T>& value, long)
			{
				T val;
				const bool result = Do(string, val, 0);
				if (result)
					value = val;

				return result;
			}
		};

		inline bool TryReadArgument(const std::string& string, size_t index)
		{
			if (index == std::numeric_limits<size_t>::max() - 1)
				return true;

			STINGRAYKIT_THROW(IndexOutOfRangeException());
		}

		template < typename T0, typename... Ts >
		bool TryReadArgument(const std::string& string, size_t index, T0& p0, Ts&... args)
		{
			if (index == std::numeric_limits<size_t>::max() - 1)
				return true;

			if (index)
				return TryReadArgument(string, index - 1, args...);
			else
				return FromStringReader::Do(string, p0, 0);
		}

	}


	template < typename... Ts >
	inline bool StringParse(const std::string& string, const std::string& format, Ts&... args)
	{
		std::deque<variant<TypeList<std::string, size_t>>> tokens;
		std::string::size_type startPos = 0;
		std::string::size_type currentPos = 0;

		do
		{
			const std::string::size_type startMarkerPos = format.find_first_of('%', currentPos);
			if (startMarkerPos == std::string::npos)
				break;

			const std::string::size_type endMarkerPos = format.find_first_of('%', startMarkerPos + 1);
			if (endMarkerPos == std::string::npos)
				return false;

			currentPos = endMarkerPos + 1;

			if (endMarkerPos - startMarkerPos > 1)
			{
				const std::string substr = format.substr(startPos, startMarkerPos - startPos);
				const std::string indexStr = format.substr(startMarkerPos + 1, endMarkerPos - startMarkerPos - 1);

				size_t index = 0;
				if (indexStr == "_")
					index = std::numeric_limits<size_t>::max();
				else
				{
					try
					{ index = FromString<size_t>(indexStr); }
					catch (const std::exception& ex)
					{ continue; }
				}

				if (!substr.empty())
					tokens.push_back(substr);

				tokens.push_back(index);

				startPos = currentPos;
			}
		}
		while (currentPos < format.length());

		if (startPos < format.length())
			tokens.push_back(format.substr(startPos));

		size_t index = 0;
		std::string::size_type currentStringPos = 0;

		while (!tokens.empty() && currentStringPos < string.length())
		{
			if (tokens.front().contains<size_t>())
			{
				index = tokens.front().get<size_t>();
				tokens.pop_front();
				continue;
			}

			const std::string substr = variant_get<std::string>(tokens.front());
			tokens.pop_front();

			const std::string::size_type substrPos = string.find(substr, currentStringPos);
			if (substrPos == std::string::npos)
				return false;

			if (index)
			{
				if (substrPos <= currentStringPos
						|| !Detail::TryReadArgument(string.substr(currentStringPos, substrPos - currentStringPos), index - 1, args...))
					return false;

				index = 0;
			}

			currentStringPos = substrPos + substr.length();
		}

		return tokens.empty()
				&& (index ?
						Detail::TryReadArgument(string.substr(currentStringPos), index - 1, args...) :
						currentStringPos >= string.length());
	}


	template < typename T, typename ConvertFunc >
	Detail::ParseProxy<T, ConvertFunc> MakeParseProxy(T& value, const ConvertFunc& convert)
	{ return Detail::ParseProxy<T, ConvertFunc>(value, convert); }

}

#endif
