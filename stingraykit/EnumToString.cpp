// Copyright (c) 2011 - 2022, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/EnumToString.h>

#include <map>
#include <sstream>

#include <stingraykit/exception.h>

namespace stingray
{
	namespace Detail
	{
		struct EnumToStringMapBase::Impl
		{
			typedef std::map<int, std::string>			EnumToStrMap;
			typedef std::map<std::string, int>			StrToEnumMap;
			typedef std::vector<int>					EnumValuesVec;

			EnumToStrMap	_enumToStr;
			StrToEnumMap	_strToEnum;
			EnumValuesVec	_values;

			const EnumValuesVec& GetEnumValues() { return _values; }
			std::string EnumToString(int val);
			int EnumFromString(const std::string& str);
			void Init(const Detail::EnumValueHolder* valuesBegin, const Detail::EnumValueHolder* valuesEnd, const std::string& str);

			static bool IsWhitespace(char c)
			{ return c == ' ' || c == '\t' || c == '\n' || c == '\r'; }
		};


		std::string EnumToStringMapBase::Impl::EnumToString(int val)
		{
			EnumToStrMap::const_iterator it = _enumToStr.find(val);
			if (it != _enumToStr.end())
				return it->second;

			s32 flagged_val = 0;
			std::string result;
			for (EnumToStrMap::const_iterator it = _enumToStr.begin(); it != _enumToStr.end(); ++it)
			{
				if (((s32)it->first & val) != 0 &&
					((s32)it->first & ~val) == 0 &&
					((s32)it->first & ~flagged_val) != 0)
				{
					flagged_val |= (s32)it->first;
					if (!result.empty())
						result += "|";
					result += it->second;
				}

				if (flagged_val == val)
					break;
			}

			if (flagged_val != val)
			{
				string_ostream s;
				s << (unsigned)val;
				return s.str();
			}

			return result;
		}


		int EnumToStringMapBase::Impl::EnumFromString(const std::string& str)
		{
			{
				std::string::const_iterator s_it = str.begin();
				while (s_it != str.end() && IsWhitespace(*s_it))
					++s_it;

				if (s_it != str.end() && *s_it >= '0' && *s_it <= '9')
				{
					std::stringstream s(str);
					unsigned val;
					s >> val;
					if (s.fail())
						STINGRAYKIT_THROW("Cannot parse enum class value: '" + str + "'!");
					while (!s.eof())
					{
						char c = 0;
						s >> c;
						if (!s.eof() && !IsWhitespace(c))
							STINGRAYKIT_THROW("Cannot parse enum class value: '" + str + "'!");
					}
					return val;
				}
			}

			StrToEnumMap::const_iterator it = _strToEnum.find(str);
			if (it != _strToEnum.end())
				return it->second;

			bool has_nonwhitespace_chars = false;
			s32 result = 0;
			std::string bit_val;
			for (std::string::const_iterator s_it = str.begin(); s_it != str.end(); ++s_it)
			{
				for (; s_it != str.end() && IsWhitespace(*s_it); ++s_it);
				if (s_it != str.end())
					has_nonwhitespace_chars = true;
				for (; s_it != str.end() && !IsWhitespace(*s_it) && *s_it != '|'; ++s_it)
					bit_val += *s_it;
				for (; s_it != str.end() && IsWhitespace(*s_it); ++s_it);

				if (!bit_val.empty())
				{
					it = _strToEnum.find(bit_val);
					if (it == _strToEnum.end())
						STINGRAYKIT_THROW("Cannot parse enum class value: '" + str + "'!");

					bit_val.clear();
					result |= (s32)it->second;
				}

				if (s_it == str.end())
					break;
			}

			if (!has_nonwhitespace_chars)
				STINGRAYKIT_THROW("Cannot parse enum class value: '" + str + "'!");

			return result;
		}

		void EnumToStringMapBase::Impl::Init(const Detail::EnumValueHolder* valuesBegin, const Detail::EnumValueHolder* valuesEnd, const std::string& str)
		{
			int cur_value = 0;
			for (; valuesBegin != valuesEnd; ++valuesBegin, ++cur_value)
			{
				if (valuesBegin->Val != Detail::EnumValueHolder::Uninitialized)
					cur_value = valuesBegin->Val;

				_values.push_back(cur_value);
			}

			std::string current_name;
			std::string::const_iterator s_it = str.begin();
			EnumValuesVec::const_iterator v_it = _values.begin();
			for (; v_it != _values.end(); ++v_it)
			{
				if (s_it == str.end())
					break;

				for (; s_it != str.end() && IsWhitespace(*s_it); ++s_it);
				for (; s_it != str.end() && !IsWhitespace(*s_it) && *s_it != ',' && *s_it != '='; ++s_it)
					current_name += *s_it;
				for (; s_it != str.end() && *s_it != ','; ++s_it);

				if (s_it != str.end())
					++s_it; // ','

				_enumToStr.insert(EnumToStrMap::value_type(*v_it, current_name));
				_strToEnum.insert(StrToEnumMap::value_type(current_name, *v_it));
				current_name.clear();
			}

			if (v_it != _values.end())
				STINGRAYKIT_THROW("Internal error in EnumToStringMap, enum values: \"" + str + "\"");
		}

		EnumToStringMapBase::EnumToStringMapBase() : _impl(new Impl())
		{}

		EnumToStringMapBase::~EnumToStringMapBase()
		{ CheckedDelete(_impl); }

		void EnumToStringMapBase::DoInit(const Detail::EnumValueHolder* valuesBegin, const Detail::EnumValueHolder* valuesEnd, const char* str)
		{ _impl->Init(valuesBegin, valuesEnd, str); }

		const EnumToStringMapBase::EnumValuesVec& EnumToStringMapBase::DoGetEnumValues() const
		{ return _impl->GetEnumValues(); }

		std::string EnumToStringMapBase::DoEnumToString(int val)
		{ return _impl->EnumToString(val); }

		int EnumToStringMapBase::DoEnumFromString(const std::string& str)
		{ return _impl->EnumFromString(str); }
	}
}
