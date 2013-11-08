#ifndef STINGRAY_TOOLKIT_STRINGPARSE_H
#define STINGRAY_TOOLKIT_STRINGPARSE_H


#include <sstream>

#include <stingray/toolkit/exception.h>


namespace stingray
{

	namespace Detail
	{

		template < typename T >
		bool TryRead(std::istringstream& stream, T& value)
		{ return stream >> value; }

		template < typename Arguments >
		struct ArgumentReader
		{
			static bool TryRead(std::istringstream& stream, const Tuple<Arguments>& arguments, size_t index)
			{
				if (index)
					return ArgumentReader<typename Arguments::Next>::TryRead(stream, arguments.GetTail(), index - 1);
				else
					return Detail::TryRead(stream, arguments.GetHead());
			}
		};

		template<>
		struct ArgumentReader<TypeListEndNode>
		{
			static bool TryRead(std::istringstream&, const Tuple<TypeListEndNode>&, size_t)
			{ TOOLKIT_THROW(IndexOutOfRangeException()); }
		};

		template < typename Arguments >
		bool StringParse(const std::string& string, const std::string& format, const Tuple<Arguments>& arguments)
		{
			std::istringstream string_stream(string);
			std::istringstream format_stream(format);

			while (format_stream)
			{
				const char string_character = string_stream.get();
				const char format_character = format_stream.get();

				if (string_character == format_character)
					continue;

				if (format_character != '%')
					return false;

				size_t index;
				if (!TryRead(format_stream, index))
					return false;

				string_stream.unget();

				if (!ArgumentReader<Arguments>::TryRead(string_stream, arguments, index - 1))
					return false;

				if (format_stream.get() != '%')
					return false;
			}

			return true;
		}

	}

#define DETAIL_DEFINE_STRING_PARSE(N_, TypesDecl_, TypesUsage_, ArgumentsDecl_, ArgumentsUsage_) \
	template < TypesDecl_ > \
	bool StringParse(const std::string& string, const std::string& format, ArgumentsDecl_) \
	{ return Detail::StringParse(string, format, Tuple<TypeList_##N_<TypesUsage_> >(ArgumentsUsage_)); }

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
