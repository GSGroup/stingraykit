#include <stingray/toolkit/EnumToString.h>
#include <stingray/toolkit/exception.h>

namespace stingray
{
	namespace Detail
	{
		void EnumToStringMap_throw(const std::string& msg) { TOOLKIT_THROW(msg); }

		std::string EnumToStringMapImpl::EnumToString(const EnumToStrMap& m, int val)
		{
			EnumToStrMap::const_iterator it = m.find(val);
			if (it != m.end())
				return it->second;

			s32 flagged_val = 0;
			std::string result;
			for (EnumToStrMap::const_iterator it = m.begin(); it != m.end(); ++it)
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
				std::stringstream s;
				s << (unsigned)val;
				return s.str();
			}

			return result;
		}

		int EnumToStringMapImpl::EnumFromString(const StrToEnumMap& m, const std::string& str)
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
						EnumToStringMap_throw("Cannot parse enum class value: '" + str + "'!");
					while (!s.eof())
					{
						char c = 0;
						s >> c;
						if (!s.eof() && !IsWhitespace(c))
							EnumToStringMap_throw("Cannot parse enum class value: '" + str + "'!");
					}
					return val;
				}
			}

			StrToEnumMap::const_iterator it = m.find(str);
			if (it != m.end())
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
					it = m.find(bit_val);
					if (it == m.end())
						EnumToStringMap_throw("Cannot parse enum class value: '" + str + "'!");

					bit_val.clear();
					result |= (s32)it->second;
				}

				if (s_it == str.end())
					break;
			}

			if (!has_nonwhitespace_chars)
				EnumToStringMap_throw("Cannot parse enum class value: '" + str + "'!");

			return result;
		}

		void EnumToStringMapImpl::Init(const EnumValuesVec& values, EnumToStrMap& enumToStr, StrToEnumMap& strToEnum, const std::string& str)
		{
			std::string current_name;
			std::string::const_iterator s_it = str.begin();
			EnumValuesVec::const_iterator v_it = values.begin();
			for (; v_it != values.end(); ++v_it)
			{
				if (s_it == str.end())
					break;

				for (; s_it != str.end() && IsWhitespace(*s_it); ++s_it);
				for (; s_it != str.end() && !IsWhitespace(*s_it) && *s_it != ','; ++s_it)
					current_name += *s_it;
				for (; s_it != str.end() && *s_it != ','; ++s_it);

				if (s_it != str.end())
					++s_it; // ','

				enumToStr.insert(EnumToStrMap::value_type(*v_it, current_name));
				strToEnum.insert(StrToEnumMap::value_type(current_name, *v_it));
				current_name.clear();
			}

			if (v_it != values.end())
				EnumToStringMap_throw("Internal error in EnumToStringMap!");
		}


		void EnumToStringMapBase::DoInit(const Detail::EnumValueHolder* valuesBegin, const Detail::EnumValueHolder* valuesEnd, const char* str, bool& initialized)
		{
			if (!initialized)
			{
				EnumValuesVec _values;
				int cur_value = 0;
				for (; valuesBegin != valuesEnd; ++valuesBegin, ++cur_value)
				{
					if (valuesBegin->Val != Detail::EnumValueHolder::Uninitialized)
						cur_value = valuesBegin->Val;

					_values.push_back(cur_value);
				}

				EnumToStringMapImpl::Init(_values, _enumToStr, _strToEnum, str);

				initialized = true;
			}
		}
	}

}
