#ifndef STINGRAYKIT_STRING_STRINGPARSE_H
#define STINGRAYKIT_STRING_STRINGPARSE_H

// Copyright (c) 2011 - 2019, GS Group, https://github.com/GSGroup
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
				try { value = FromString<T>(str); }
				catch (const std::exception&) { return false; }
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
				return (stream >> value).eof();
			}

			template < typename T, typename ConvertFunc >
			static bool Do(const std::string& str, ParseProxy<T, ConvertFunc>& adapter, long)
			{
				try { adapter.Parse(str); }
				catch (const std::exception& ex) { return false; }
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
		std::deque<variant<TypeList<std::string, size_t>> > tokens;
		std::string::size_type start_pos = 0, current_pos = 0;
		do
		{
			std::string::size_type start_marker_pos = format.find_first_of('%', current_pos);
			if (start_marker_pos == std::string::npos)
				break;
			std::string::size_type end_marker_pos = format.find_first_of('%', start_marker_pos + 1);
			if (end_marker_pos == std::string::npos)
				return false;

			current_pos = end_marker_pos + 1;

			if (end_marker_pos - start_marker_pos > 1)
			{
				std::string substr(format, start_pos, start_marker_pos - start_pos);
				try
				{
					const std::string index_str = std::string(format, start_marker_pos + 1, end_marker_pos - start_marker_pos - 1);
					size_t index = index_str == "_"? std::numeric_limits<size_t>::max() : FromString<size_t>(index_str);
					if (!substr.empty())
						tokens.push_back(substr);
					tokens.push_back(index);
				}
				catch (const std::exception& ex) { continue; }
				start_pos = current_pos;
			}
		}
		while (current_pos < format.length());

		if (start_pos < format.length())
			tokens.push_back(std::string(format, start_pos));

		size_t index = 0;
		std::string::size_type current_string_pos = 0;
		while (!tokens.empty() && current_string_pos < string.length())
		{
			if (tokens.front().contains<size_t>())
			{
				index = tokens.front().get<size_t>();
				tokens.pop_front();
				continue;
			}

			std::string substr = variant_get<std::string>(tokens.front());
			tokens.pop_front();
			std::string::size_type substr_pos = string.find(substr, current_string_pos);
			if (substr_pos == std::string::npos)
				return false;

			if (index)
			{
				if (!(substr_pos - current_string_pos > 0 && Detail::TryReadArgument(std::string(string, current_string_pos, substr_pos - current_string_pos), index - 1, args...)))
					return false;
				index = 0;
			}
			current_string_pos = substr_pos + substr.length();
		}

		return tokens.empty() && (index ? Detail::TryReadArgument(std::string(string.begin() + current_string_pos, string.end()), index - 1, args...) : !(current_string_pos < string.length()));
	}


	template < typename T, typename ConvertFunc >
	Detail::ParseProxy<T, ConvertFunc> MakeParseProxy(T& value, const ConvertFunc& convert)
	{ return Detail::ParseProxy<T, ConvertFunc>(value, convert); }

}

#endif
