#ifndef STINGRAY_TOOLKIT_STRINGPARSE_H
#define STINGRAY_TOOLKIT_STRINGPARSE_H


#include <sstream>

#include <stingray/toolkit/exception.h>
#include <stingray/toolkit/Tuple.h>


namespace stingray
{

	namespace Detail
	{

		template < typename T >
		bool TryRead(const std::string& string, T& value)
		{
			std::istringstream stream(string);
			return stream >> value;
		}

		bool TryRead(const std::string& string, std::string& value)
		{
			value = string;
			return true;
		}

		struct ArgumentReader
		{
			template < typename Arguments >
			static bool TryRead(const std::string& string, const Tuple<Arguments>& arguments, size_t index)
			{
				if (index)
					return ArgumentReader::TryRead(string, arguments.GetTail(), index - 1);
				else
					return Detail::TryRead(string, arguments.GetHead());
			}

			static bool TryRead(const std::string&, const Tuple<TypeListEndNode>&, size_t)
			{ TOOLKIT_THROW(IndexOutOfRangeException()); }
		};

		template < typename Arguments >
		bool StringParseImpl(const std::string& string, const std::string& format, const Tuple<Arguments>& arguments)
		{
			std::string::const_iterator string_iterator = string.begin();
			std::string::const_iterator format_iterator = format.begin();

			while (string_iterator != string.end() && format_iterator != format.end())
			{
				if (*string_iterator == *format_iterator)
				{
					++string_iterator;
					++format_iterator;
					continue;
				}

				if (*format_iterator != '%')
					return false;

				const std::string::const_iterator index_begin = next(format_iterator);

				const std::string::const_iterator index_end = std::find(index_begin, format.end(), '%');
				if (index_end == format.end())
					return false;

				size_t index;
				if (!TryRead(std::string(index_begin, index_end), index))
					return false;

				format_iterator = next(index_end);

				const std::string::const_iterator argument_end = format_iterator == format.end()? string.end() : std::find(next(string_iterator), string.end(), *format_iterator);

				if(!ArgumentReader::TryRead(std::string(string_iterator, argument_end), arguments, index - 1))
					return false;

				string_iterator = argument_end;
			}

			return string_iterator == string.end() && format_iterator == format.end();
		}

	}

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
