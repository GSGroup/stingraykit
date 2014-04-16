#ifndef STINGRAY_TOOLKIT_STRINGPARSE_H
#define STINGRAY_TOOLKIT_STRINGPARSE_H


#include <deque>
#include <limits>
#include <sstream>

#include <stingray/toolkit/exception.h>
#include <stingray/toolkit/iterators.h>
#include <stingray/toolkit/Tuple.h>
#include <stingray/toolkit/variant.h>


namespace stingray
{

	namespace Detail
	{

		template < typename T, bool HasFromStringMethod = HasMethod_FromString<T>::Value >
		struct FromStringImpl
		{
			static bool Do(const std::string& str, T& value)
			{
				try { value = T::FromString(str); }
				catch (const std::exception&) { return false; }
				return true;
			}
		};

		template < typename T >
		struct FromStringImpl<T, false>
		{
			static bool Do(const std::string& str, T& value)
			{
				std::istringstream stream(str);
				return (stream >> value).eof();
			}
		};


		template < typename T >
		bool TryRead(const std::string& string, T& value)
		{ return FromStringImpl<T>::Do(string, value); }

		inline bool TryRead(const std::string& string, u8& value)
		{
			try { value = FromString<u8>(string); }
			catch (const std::exception&) { return false; }
			return true;
		}

		inline bool TryRead(const std::string& string, char& value)
		{
			if (string.length() != 1)
				return false;
			value = string[0];
			return true;
		}

		inline bool TryRead(const std::string& string, std::string& value)
		{
			value = string;
			return true;
		}

		struct ArgumentReader
		{
			template < typename Arguments >
			static bool TryRead(const std::string& string, const Tuple<Arguments>& arguments, size_t index)
			{
				if (index == std::numeric_limits<size_t>::max() - 1)
					return true;

				if (index)
					return ArgumentReader::TryRead(string, arguments.GetTail(), index - 1);
				else
					return Detail::TryRead(string, arguments.GetHead());
			}

			static bool TryRead(const std::string&, const Tuple<TypeListEndNode>&, size_t index)
			{
				if (index == std::numeric_limits<size_t>::max() - 1)
					return true;

				TOOLKIT_THROW(IndexOutOfRangeException());
			}
		};

		template < typename Arguments >
		bool StringParseImpl(const std::string& string, const std::string& format, const Tuple<Arguments>& arguments)
		{
			std::deque<variant<TypeList<std::string, size_t>::type > > tokens;
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

			if (start_pos < format.length() - 1)
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
					if (!(substr_pos - current_string_pos > 0 && ArgumentReader::TryRead(std::string(string, current_string_pos, substr_pos - current_string_pos), arguments, index - 1)))
						return false;
					index = 0;
				}
				current_string_pos = substr_pos + substr.length();
			}

			return tokens.empty() && (index ? ArgumentReader::TryRead(std::string(string.begin() + current_string_pos, string.end()), arguments, index - 1) : !(current_string_pos < string.length()));
		}

	}

	inline bool StringParse(const std::string& string, const std::string& format)
	{ return Detail::StringParseImpl(string, format, Tuple<TypeList_0>()); }

#define DETAIL_DEFINE_STRING_PARSE(N_, TypesDecl_, TypesUsage_, ArgumentsDecl_, ArgumentsUsage_) \
	template < TypesDecl_ > \
	bool StringParse(const std::string& string, const std::string& format, ArgumentsDecl_) \
	{ return Detail::StringParseImpl(string, format, Tuple<TypeList_##N_<TypesUsage_> >(ArgumentsUsage_)); }

#define TY typename

	DETAIL_DEFINE_STRING_PARSE(1, TY T1, T1&, T1& p1, p1)
	DETAIL_DEFINE_STRING_PARSE(2, MK_PARAM(TY T1, TY T2), MK_PARAM(T1&, T2&), MK_PARAM(T1& a1, T2& a2), MK_PARAM(a1, a2))
	DETAIL_DEFINE_STRING_PARSE(3, MK_PARAM(TY T1, TY T2, TY T3), MK_PARAM(T1&, T2&, T3&), MK_PARAM(T1& a1, T2& a2, T3& a3), MK_PARAM(a1, a2, a3))
	DETAIL_DEFINE_STRING_PARSE(4, MK_PARAM(TY T1, TY T2, TY T3, TY T4), MK_PARAM(T1&, T2&, T3&, T4&), MK_PARAM(T1& a1, T2& a2, T3& a3, T4& a4), MK_PARAM(a1, a2, a3, a4))
	DETAIL_DEFINE_STRING_PARSE(5, MK_PARAM(TY T1, TY T2, TY T3, TY T4, TY T5), MK_PARAM(T1&, T2&, T3&, T4&, T5&), MK_PARAM(T1& a1, T2& a2, T3& a3, T4& a4, T5& a5), MK_PARAM(a1, a2, a3, a4, a5))

#undef TY

#undef DETAIL_DEFINE_STRING_PARSE


}


#endif
