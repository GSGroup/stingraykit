// Copyright (c) 2011 - 2024, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/Enum.h>

#include <stingraykit/string/ToString.h>

#include <map>
#include <sstream>

namespace stingray
{
	namespace Detail
	{

		namespace
		{

			bool IsWhitespace(char c)
			{ return c == ' ' || c == '\t' || c == '\n' || c == '\r'; }

		}

		struct EnumToStringMapBase::Impl
		{
			using EnumToStrMap = std::map<int, std::string>;
			using StrToEnumMap = std::map<std::string, int>;
			using EnumValuesVec = std::vector<int>;

		private:
			EnumToStrMap		_enumToStr;
			StrToEnumMap		_strToEnum;
			EnumValuesVec		_values;

		public:
			const EnumValuesVec& GetEnumValues() const
			{ return _values; }

			std::string EnumToString(int value)
			{
				const EnumToStrMap::const_iterator it = _enumToStr.find(value);
				if (it != _enumToStr.end())
					return it->second;

				s32 flaggedValue = 0;
				std::string result;
				for (const auto& valueToStr : _enumToStr)
				{
					if (((s32)valueToStr.first & value) != 0
							&& ((s32)valueToStr.first & ~value) == 0
							&& ((s32)valueToStr.first & ~flaggedValue) != 0)
					{
						flaggedValue |= (s32)valueToStr.first;
						if (!result.empty())
							result += "|";
						result += valueToStr.second;
					}

					if (flaggedValue == value)
						break;
				}

				if (flaggedValue != value)
				{
					string_ostream s;
					s << (unsigned)value;
					return s.str();
				}

				return result;
			}

			int EnumFromString(const std::string& str)
			{
				{
					std::string::const_iterator strIt = str.begin();
					while (strIt != str.end() && IsWhitespace(*strIt))
						++strIt;

					if (strIt != str.end() && *strIt >= '0' && *strIt <= '9')
					{
						std::stringstream s(str);

						unsigned value;
						s >> value;
						STINGRAYKIT_CHECK(!s.fail(), FormatException(str));

						while (!s.eof())
						{
							char c = 0;
							s >> c;
							STINGRAYKIT_CHECK(s.eof() || IsWhitespace(c), FormatException(str));
						}

						return value;
					}
				}

				const StrToEnumMap::const_iterator it = _strToEnum.find(str);
				if (it != _strToEnum.end())
					return it->second;

				bool hasNonwhitespaceChars = false;
				s32 result = 0;
				std::string bitValue;

				for (std::string::const_iterator strIt = str.begin(); strIt != str.end(); ++strIt)
				{
					for (; strIt != str.end() && IsWhitespace(*strIt); ++strIt);

					if (strIt != str.end())
						hasNonwhitespaceChars = true;

					for (; strIt != str.end() && !IsWhitespace(*strIt) && *strIt != '|'; ++strIt)
						bitValue += *strIt;

					for (; strIt != str.end() && IsWhitespace(*strIt); ++strIt);

					if (!bitValue.empty())
					{
						const StrToEnumMap::const_iterator it = _strToEnum.find(bitValue);
						STINGRAYKIT_CHECK(it != _strToEnum.end(), KeyNotFoundException(str));

						bitValue.clear();
						result |= (s32)it->second;
					}

					if (strIt == str.end())
						break;
				}

				STINGRAYKIT_CHECK(hasNonwhitespaceChars, FormatException(str));
				return result;
			}

			void Init(const EnumValueHolder* valuesBegin, const EnumValueHolder* valuesEnd, const std::string& str)
			{
				int currentValue = 0;
				for (; valuesBegin != valuesEnd; ++valuesBegin, ++currentValue)
				{
					if (valuesBegin->Value != EnumValueHolder::Uninitialized)
						currentValue = valuesBegin->Value;

					_values.push_back(currentValue);
				}

				std::string currentName;
				std::string::const_iterator strIt = str.begin();
				EnumValuesVec::const_iterator valueIt = _values.begin();

				for (; valueIt != _values.end(); ++valueIt)
				{
					if (strIt == str.end())
						break;

					for (; strIt != str.end() && IsWhitespace(*strIt); ++strIt);

					for (; strIt != str.end() && !IsWhitespace(*strIt) && *strIt != ',' && *strIt != '='; ++strIt)
						currentName += *strIt;

					for (; strIt != str.end() && *strIt != ','; ++strIt);

					if (strIt != str.end())
						++strIt; // ','

					_enumToStr.emplace(*valueIt, currentName);
					_strToEnum.emplace(currentName, *valueIt);
					currentName.clear();
				}

				STINGRAYKIT_CHECK(valueIt == _values.end(), LogicException(StringBuilder() % "Invalid enum values: '" % str % "'"));
			}
		};

		void EnumToStringMapBase::Init(const EnumValueHolder* valuesBegin, const EnumValueHolder* valuesEnd, const char* str)
		{ _impl->Init(valuesBegin, valuesEnd, str); }

		const EnumToStringMapBase::EnumValuesVec& EnumToStringMapBase::GetEnumValues() const
		{ return _impl->GetEnumValues(); }

		std::string EnumToStringMapBase::EnumToString(int value)
		{ return _impl->EnumToString(value); }

		int EnumToStringMapBase::EnumFromString(const std::string& str)
		{ return _impl->EnumFromString(str); }

		EnumToStringMapBase::EnumToStringMapBase()
			:	_impl(new Impl())
		{ }

		EnumToStringMapBase::~EnumToStringMapBase()
		{ CheckedDelete(_impl); }

	}
}
