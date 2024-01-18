// Copyright (c) 2011 - 2024, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/collection/EnumerableHelpers.h>
#include <stingraykit/collection/Range.h>
#include <stingraykit/string/ToString.h>

#include <gtest/gtest.h>

#include <deque>
#include <list>

using namespace stingray;

TEST(ToStringTest, StringOstream)
{
	{
		string_ostream str;
		bool b = false;
		str << b;
		ASSERT_EQ(str.str(), "false");
		str << true;
		ASSERT_EQ(str.str(), "falsetrue");
	}

	{
		string_ostream str;
		char b = '@';
		str << b;
		ASSERT_EQ(str.str(), "@");
	}

	{
		string_ostream str;
		unsigned long v = 1234567890;
		str << v;
		ASSERT_EQ(str.str(), "1234567890");
	}
	{
		string_ostream str;
		std::string v = "REDRUM";
		str << v;
		ASSERT_EQ(str.str(), "REDRUM");
	}

	{
		string_ostream str;
		str << "ALL";
		str << "WORKANDNOPLAYMAKESJACK";
		str << "ADULLBOY";
		ASSERT_EQ(str.str(), "ALLWORKANDNOPLAYMAKESJACKADULLBOY");
	}
}


namespace
{

	struct TestEnum final
	{
		STINGRAYKIT_ENUM_VALUES
		(
			Test = 1,
			Enum_ = 2,
			Value = 3
		);

		STINGRAYKIT_DECLARE_ENUM_CLASS(TestEnum);
	};

	struct ArgileEnum final
	{
		enum Enum
		{
			None = 4,
			Value1 = 5,
			Value2 = 6
		};

	private:
		Enum	_value;

	public:
		ArgileEnum(Enum value = None) : _value(value) { }

		ArgileEnum& operator = (Enum value) { _value = value; return *this; }

		operator Enum () const { return _value; }
	};

}


TEST(ToStringTest, FromString)
{
	{
		ASSERT_EQ(FromString<TestEnum>("Test"), TestEnum(TestEnum::Test));
		ASSERT_EQ(FromString<TestEnum>("Enum_"), TestEnum(TestEnum::Enum_));
		ASSERT_EQ(FromString<TestEnum>("Value"), TestEnum(TestEnum::Value));
		ASSERT_ANY_THROW(FromString<TestEnum>(""));
		ASSERT_ANY_THROW(FromString<TestEnum>("Unknown"));
	}

	{
		ASSERT_ANY_THROW(FromString<bool>(std::string("-1")));
		ASSERT_EQ(FromString<bool>(std::string("0")), false);
		ASSERT_EQ(FromString<bool>(std::string("1")), true);
		ASSERT_ANY_THROW(FromString<bool>(std::string("2")));
		ASSERT_ANY_THROW(FromString<bool>(std::string("3")));
		ASSERT_ANY_THROW(FromString<bool>(std::string("9")));
		ASSERT_ANY_THROW(FromString<bool>(std::string("10")));
		ASSERT_ANY_THROW(FromString<bool>(std::string("11")));
	}

	{
		ASSERT_EQ(FromString<int>(std::string("-1234567890")), -1234567890);
		ASSERT_EQ(FromString<int>(std::string("-123")), -123);
		ASSERT_EQ(FromString<int>(std::string("-00012")), -12);
		ASSERT_EQ(FromString<int>(std::string("-12")), -12);
		ASSERT_EQ(FromString<int>(std::string("-0001")), -1);
		ASSERT_EQ(FromString<int>(std::string("-1")), -1);
		ASSERT_EQ(FromString<int>(std::string("-000")), 0);
		ASSERT_EQ(FromString<int>(std::string("-0")), 0);
		ASSERT_EQ(FromString<int>(std::string("")), 0);
		ASSERT_EQ(FromString<int>(std::string("0")), 0);
		ASSERT_EQ(FromString<int>(std::string("000")), 0);
		ASSERT_EQ(FromString<int>(std::string("1")), 1);
		ASSERT_EQ(FromString<int>(std::string("0001")), 1);
		ASSERT_EQ(FromString<int>(std::string("12")), 12);
		ASSERT_EQ(FromString<int>(std::string("00012")), 12);
		ASSERT_EQ(FromString<int>(std::string("123")), 123);
		ASSERT_EQ(FromString<int>(std::string("1234567890")), 1234567890);
	}

	{
		ASSERT_ANY_THROW(FromString<unsigned>(std::string("-1234567890")));
		ASSERT_ANY_THROW(FromString<unsigned>(std::string("-123")));
		ASSERT_ANY_THROW(FromString<unsigned>(std::string("-12")));
		ASSERT_ANY_THROW(FromString<unsigned>(std::string("-1")));
		ASSERT_ANY_THROW(FromString<unsigned>(std::string("-0")));
		ASSERT_EQ(FromString<unsigned>(std::string("")), 0u);
		ASSERT_EQ(FromString<unsigned>(std::string("0")), 0u);
		ASSERT_EQ(FromString<unsigned>(std::string("000")), 0u);
		ASSERT_EQ(FromString<unsigned>(std::string("1")), 1u);
		ASSERT_EQ(FromString<unsigned>(std::string("0001")), 1u);
		ASSERT_EQ(FromString<unsigned>(std::string("12")), 12u);
		ASSERT_EQ(FromString<unsigned>(std::string("00012")), 12u);
		ASSERT_EQ(FromString<unsigned>(std::string("123")), 123u);
		ASSERT_EQ(FromString<unsigned>(std::string("1234567890")), 1234567890u);
	}

	{
		ASSERT_ANY_THROW(FromString<u8>(std::string("-257")));
		ASSERT_ANY_THROW(FromString<u8>(std::string("-256")));
		ASSERT_ANY_THROW(FromString<u8>(std::string("-255")));
		ASSERT_ANY_THROW(FromString<u8>(std::string("-129")));
		ASSERT_ANY_THROW(FromString<u8>(std::string("-128")));
		ASSERT_ANY_THROW(FromString<u8>(std::string("-127")));
		ASSERT_ANY_THROW(FromString<u8>(std::string("-12")));
		ASSERT_ANY_THROW(FromString<u8>(std::string("-1")));
		ASSERT_EQ(FromString<u8>(std::string("0")), 0u);
		ASSERT_EQ(FromString<u8>(std::string("1")), 1u);
		ASSERT_EQ(FromString<u8>(std::string("12")), 12u);
		ASSERT_EQ(FromString<u8>(std::string("123")), 123u);
		ASSERT_EQ(FromString<u8>(std::string("254")), 254u);
		ASSERT_EQ(FromString<u8>(std::string("255")), 255u);
		ASSERT_ANY_THROW(FromString<u8>(std::string("256")));
		ASSERT_ANY_THROW(FromString<u8>(std::string("257")));

		ASSERT_ANY_THROW(FromString<s8>(std::string("-257")));
		ASSERT_ANY_THROW(FromString<s8>(std::string("-256")));
		ASSERT_ANY_THROW(FromString<s8>(std::string("-255")));
		ASSERT_ANY_THROW(FromString<s8>(std::string("-129")));
		ASSERT_EQ(FromString<s8>(std::string("-128")), -128);
		ASSERT_EQ(FromString<s8>(std::string("-127")), -127);
		ASSERT_EQ(FromString<s8>(std::string("-12")), -12);
		ASSERT_EQ(FromString<s8>(std::string("-1")), -1);
		ASSERT_EQ(FromString<s8>(std::string("0")), 0);
		ASSERT_EQ(FromString<s8>(std::string("12")), 12);
		ASSERT_EQ(FromString<s8>(std::string("126")), 126);
		ASSERT_EQ(FromString<s8>(std::string("127")), 127);
		ASSERT_ANY_THROW(FromString<s8>(std::string("128")));
		ASSERT_ANY_THROW(FromString<s8>(std::string("129")));
		ASSERT_ANY_THROW(FromString<s8>(std::string("255")));
		ASSERT_ANY_THROW(FromString<s8>(std::string("256")));
		ASSERT_ANY_THROW(FromString<s8>(std::string("257")));
	}

	{
		ASSERT_ANY_THROW(FromString<u16>(std::string("-65537")));
		ASSERT_ANY_THROW(FromString<u16>(std::string("-65536")));
		ASSERT_ANY_THROW(FromString<u16>(std::string("-65535")));
		ASSERT_ANY_THROW(FromString<u16>(std::string("-32769")));
		ASSERT_ANY_THROW(FromString<u16>(std::string("-32768")));
		ASSERT_ANY_THROW(FromString<u16>(std::string("-32767")));
		ASSERT_ANY_THROW(FromString<u16>(std::string("-12")));
		ASSERT_ANY_THROW(FromString<u16>(std::string("-1")));
		ASSERT_EQ(FromString<u16>(std::string("0")), 0u);
		ASSERT_EQ(FromString<u16>(std::string("1")), 1u);
		ASSERT_EQ(FromString<u16>(std::string("12")), 12u);
		ASSERT_EQ(FromString<u16>(std::string("123")), 123u);
		ASSERT_EQ(FromString<u16>(std::string("65534")), 65534u);
		ASSERT_EQ(FromString<u16>(std::string("65535")), 65535u);
		ASSERT_ANY_THROW(FromString<u16>(std::string("65536")));
		ASSERT_ANY_THROW(FromString<u16>(std::string("65537")));

		ASSERT_ANY_THROW(FromString<s16>(std::string("-65537")));
		ASSERT_ANY_THROW(FromString<s16>(std::string("-65536")));
		ASSERT_ANY_THROW(FromString<s16>(std::string("-65535")));
		ASSERT_ANY_THROW(FromString<s16>(std::string("-32769")));
		ASSERT_EQ(FromString<s16>(std::string("-32768")), -32768);
		ASSERT_EQ(FromString<s16>(std::string("-32767")), -32767);
		ASSERT_EQ(FromString<s16>(std::string("-123")), -123);
		ASSERT_EQ(FromString<s16>(std::string("-12")), -12);
		ASSERT_EQ(FromString<s16>(std::string("-1")), -1);
		ASSERT_EQ(FromString<s16>(std::string("0")), 0);
		ASSERT_EQ(FromString<s16>(std::string("12")), 12);
		ASSERT_EQ(FromString<s16>(std::string("123")), 123);
		ASSERT_EQ(FromString<s16>(std::string("32766")), 32766);
		ASSERT_EQ(FromString<s16>(std::string("32767")), 32767);
		ASSERT_ANY_THROW(FromString<s16>(std::string("32768")));
		ASSERT_ANY_THROW(FromString<s16>(std::string("65535")));
		ASSERT_ANY_THROW(FromString<s16>(std::string("65536")));
		ASSERT_ANY_THROW(FromString<s16>(std::string("65537")));
	}

	{
		ASSERT_ANY_THROW(FromString<u32>(std::string("-4294967297")));
		ASSERT_ANY_THROW(FromString<u32>(std::string("-4294967296")));
		ASSERT_ANY_THROW(FromString<u32>(std::string("-4294967295")));
		ASSERT_ANY_THROW(FromString<u32>(std::string("-2147483649")));
		ASSERT_ANY_THROW(FromString<u32>(std::string("-2147483648")));
		ASSERT_ANY_THROW(FromString<u32>(std::string("-2147483647")));
		ASSERT_ANY_THROW(FromString<u32>(std::string("-12")));
		ASSERT_ANY_THROW(FromString<u32>(std::string("-1")));
		ASSERT_EQ(FromString<u32>(std::string("0")), 0u);
		ASSERT_EQ(FromString<u32>(std::string("1")), 1u);
		ASSERT_EQ(FromString<u32>(std::string("12")), 12u);
		ASSERT_EQ(FromString<u32>(std::string("123")), 123u);
		ASSERT_EQ(FromString<u32>(std::string("4294967294")), 4294967294u);
		ASSERT_EQ(FromString<u32>(std::string("4294967295")), 4294967295u);
		ASSERT_ANY_THROW(FromString<u32>(std::string("4294967296")));
		ASSERT_ANY_THROW(FromString<u32>(std::string("4294967297")));

		ASSERT_ANY_THROW(FromString<s32>(std::string("-4294967297")));
		ASSERT_ANY_THROW(FromString<s32>(std::string("-4294967296")));
		ASSERT_ANY_THROW(FromString<s32>(std::string("-4294967295")));
		ASSERT_ANY_THROW(FromString<s32>(std::string("-2147483649")));
		ASSERT_EQ(FromString<s32>(std::string("-2147483648")), -2147483648);
		ASSERT_EQ(FromString<s32>(std::string("-2147483647")), -2147483647);
		ASSERT_EQ(FromString<s32>(std::string("-123")), -123);
		ASSERT_EQ(FromString<s32>(std::string("-12")), -12);
		ASSERT_EQ(FromString<s32>(std::string("-1")), -1);
		ASSERT_EQ(FromString<s32>(std::string("0")), 0);
		ASSERT_EQ(FromString<s32>(std::string("12")), 12);
		ASSERT_EQ(FromString<s32>(std::string("123")), 123);
		ASSERT_EQ(FromString<s32>(std::string("2147483646")), 2147483646);
		ASSERT_EQ(FromString<s32>(std::string("2147483647")), 2147483647);
		ASSERT_ANY_THROW(FromString<s32>(std::string("2147483648")));
		ASSERT_ANY_THROW(FromString<s32>(std::string("4294967295")));
		ASSERT_ANY_THROW(FromString<s32>(std::string("4294967296")));
		ASSERT_ANY_THROW(FromString<s32>(std::string("4294967297")));
	}

	{
		ASSERT_ANY_THROW(FromString<u64>(std::string("-18446744073709551617")));
		ASSERT_ANY_THROW(FromString<u64>(std::string("-18446744073709551616")));
		ASSERT_ANY_THROW(FromString<u64>(std::string("-18446744073709551615")));
		ASSERT_ANY_THROW(FromString<u64>(std::string("-9223372036854775809")));
		ASSERT_ANY_THROW(FromString<u64>(std::string("-9223372036854775808")));
		ASSERT_ANY_THROW(FromString<u64>(std::string("-9223372036854775807")));
		ASSERT_ANY_THROW(FromString<u64>(std::string("-12")));
		ASSERT_ANY_THROW(FromString<u64>(std::string("-1")));
		ASSERT_EQ(FromString<u64>(std::string("0")), 0u);
		ASSERT_EQ(FromString<u64>(std::string("1")), 1u);
		ASSERT_EQ(FromString<u64>(std::string("12")), 12u);
		ASSERT_EQ(FromString<u64>(std::string("123")), 123u);
		ASSERT_EQ(FromString<u64>(std::string("18446744073709551614")), 18446744073709551614ull);
		ASSERT_EQ(FromString<u64>(std::string("18446744073709551615")), 18446744073709551615ull);
		ASSERT_ANY_THROW(FromString<u64>(std::string("18446744073709551616")));
		ASSERT_ANY_THROW(FromString<u64>(std::string("18446744073709551617")));

		ASSERT_ANY_THROW(FromString<s64>(std::string("-18446744073709551617")));
		ASSERT_ANY_THROW(FromString<s64>(std::string("-18446744073709551616")));
		ASSERT_ANY_THROW(FromString<s64>(std::string("-18446744073709551615")));
		ASSERT_ANY_THROW(FromString<s64>(std::string("-9223372036854775809")));
		ASSERT_EQ(FromString<s64>(std::string("-9223372036854775808")), -9223372036854775807ll - 1);
		ASSERT_EQ(FromString<s64>(std::string("-9223372036854775807")), -9223372036854775807ll);
		ASSERT_EQ(FromString<s64>(std::string("-123")), -123);
		ASSERT_EQ(FromString<s64>(std::string("-12")), -12);
		ASSERT_EQ(FromString<s64>(std::string("-1")), -1);
		ASSERT_EQ(FromString<s64>(std::string("0")), 0);
		ASSERT_EQ(FromString<s64>(std::string("12")), 12);
		ASSERT_EQ(FromString<s64>(std::string("123")), 123);
		ASSERT_EQ(FromString<s64>(std::string("9223372036854775806")), 9223372036854775806ll);
		ASSERT_EQ(FromString<s64>(std::string("9223372036854775807")), 9223372036854775807ll);
		ASSERT_ANY_THROW(FromString<s64>(std::string("9223372036854775808")));
		ASSERT_ANY_THROW(FromString<s64>(std::string("18446744073709551615")));
		ASSERT_ANY_THROW(FromString<s64>(std::string("18446744073709551616")));
		ASSERT_ANY_THROW(FromString<s64>(std::string("18446744073709551617")));
	}
}


namespace
{

	class StringRepresentableType final
	{
	private:
		std::string		_data;

	public:
		explicit StringRepresentableType(const std::string& data) : _data(data)
		{ }

		std::string ToString() const
		{ return StringBuilder() % "StringRepresentableType { " % _data % " }"; }
	};
	STINGRAYKIT_DECLARE_PTR(StringRepresentableType);

	struct NotStringRepresentableType final
	{ };
	STINGRAYKIT_DECLARE_PTR(NotStringRepresentableType);

	struct TestException final : public std::runtime_error
	{
		explicit TestException(const std::string& message) : std::runtime_error(message)
		{ }
	};

}


TEST(ToStringTest, IsFromStringInterpretable)
{
	ASSERT_FALSE(IsFromStringInterpretable<std::vector<int>>::Value);
	ASSERT_FALSE(IsFromStringInterpretable<std::list<int>>::Value);
	ASSERT_FALSE(IsFromStringInterpretable<std::deque<int>>::Value);
	ASSERT_FALSE(IsFromStringInterpretable<std::set<int>>::Value);
	ASSERT_FALSE(IsFromStringInterpretable<std::multiset<int>>::Value);

	ASSERT_FALSE(IsFromStringInterpretable<StringRepresentableType>::Value);
	ASSERT_FALSE(IsFromStringInterpretable<StringRepresentableTypePtr>::Value);

	ASSERT_FALSE(IsFromStringInterpretable<NotStringRepresentableType>::Value);
	ASSERT_FALSE(IsFromStringInterpretable<NotStringRepresentableTypePtr>::Value);

	ASSERT_TRUE(IsFromStringInterpretable<char>::Value);
	ASSERT_TRUE(IsFromStringInterpretable<short>::Value);
	ASSERT_TRUE(IsFromStringInterpretable<int>::Value);
	ASSERT_TRUE(IsFromStringInterpretable<long>::Value);

	ASSERT_TRUE(IsFromStringInterpretable<u8>::Value);
	ASSERT_TRUE(IsFromStringInterpretable<s8>::Value);
	ASSERT_TRUE(IsFromStringInterpretable<u16>::Value);
	ASSERT_TRUE(IsFromStringInterpretable<s16>::Value);
	ASSERT_TRUE(IsFromStringInterpretable<u32>::Value);
	ASSERT_TRUE(IsFromStringInterpretable<s32>::Value);
	ASSERT_TRUE(IsFromStringInterpretable<u64>::Value);
	ASSERT_TRUE(IsFromStringInterpretable<s64>::Value);

	ASSERT_TRUE(IsFromStringInterpretable<bool>::Value);

	ASSERT_FALSE(IsFromStringInterpretable<char*>::Value);
	ASSERT_FALSE(IsFromStringInterpretable<const char*>::Value);
	ASSERT_FALSE(IsFromStringInterpretable<void*>::Value);
	ASSERT_FALSE(IsFromStringInterpretable<const void*>::Value);

	ASSERT_FALSE(IsFromStringInterpretable<float>::Value);
	ASSERT_FALSE(IsFromStringInterpretable<double>::Value);
	ASSERT_FALSE(IsFromStringInterpretable<long double>::Value);

	ASSERT_FALSE(IsFromStringInterpretable<EmptyType>::Value);
	ASSERT_FALSE(IsFromStringInterpretable<NullPtrType>::Value);

	ASSERT_TRUE(IsFromStringInterpretable<std::string>::Value);
	ASSERT_FALSE(IsFromStringInterpretable<shared_ptr<std::string>>::Value);
	ASSERT_FALSE(IsFromStringInterpretable<optional<std::string>>::Value);

	ASSERT_FALSE(IsFromStringInterpretable<shared_ptr<int>>::Value);
	ASSERT_FALSE(IsFromStringInterpretable<optional<int>>::Value);

	ASSERT_TRUE(IsFromStringInterpretable<TestEnum>::Value);
	ASSERT_FALSE(IsFromStringInterpretable<TestException>::Value);

	ASSERT_FALSE(IsFromStringInterpretable<decltype(MakeTuple())>::Value);
	ASSERT_FALSE(IsFromStringInterpretable<decltype(MakeTuple(0, ""))>::Value);
	ASSERT_FALSE(IsFromStringInterpretable<decltype(MakeTuple(0, StringRepresentableType("1")))>::Value);
	ASSERT_FALSE(IsFromStringInterpretable<decltype(MakeTuple(0, NotStringRepresentableType()))>::Value);
}


TEST(ToStringTest, IsStringRepresentable)
{
	ASSERT_TRUE(IsStringRepresentable<std::vector<int>>::Value);
	ASSERT_TRUE(IsStringRepresentable<std::list<int>>::Value);
	ASSERT_TRUE(IsStringRepresentable<std::deque<int>>::Value);
	ASSERT_TRUE(IsStringRepresentable<std::set<int>>::Value);
	ASSERT_TRUE(IsStringRepresentable<std::multiset<int>>::Value);

	ASSERT_TRUE(IsStringRepresentable<StringRepresentableType>::Value);
	ASSERT_TRUE(IsStringRepresentable<StringRepresentableTypePtr>::Value);

	ASSERT_FALSE(IsStringRepresentable<NotStringRepresentableType>::Value);
	ASSERT_FALSE(IsStringRepresentable<NotStringRepresentableTypePtr>::Value);

	ASSERT_TRUE(IsStringRepresentable<std::vector<StringRepresentableType>>::Value);
	ASSERT_TRUE(IsStringRepresentable<std::list<StringRepresentableType>>::Value);
	ASSERT_TRUE(IsStringRepresentable<std::deque<StringRepresentableType>>::Value);
	ASSERT_TRUE(IsStringRepresentable<std::set<StringRepresentableType>>::Value);
	ASSERT_TRUE(IsStringRepresentable<std::multiset<StringRepresentableType>>::Value);

	ASSERT_FALSE(IsStringRepresentable<std::vector<NotStringRepresentableType>>::Value);
	ASSERT_FALSE(IsStringRepresentable<std::list<NotStringRepresentableType>>::Value);
	ASSERT_FALSE(IsStringRepresentable<std::deque<NotStringRepresentableType>>::Value);
	ASSERT_FALSE(IsStringRepresentable<std::set<NotStringRepresentableType>>::Value);
	ASSERT_FALSE(IsStringRepresentable<std::multiset<NotStringRepresentableType>>::Value);

	{
		const bool mapIntString = IsStringRepresentable<std::map<int, std::string>>::Value;
		ASSERT_TRUE(mapIntString);
		const bool mmapIntString = IsStringRepresentable<std::multimap<int, std::string>>::Value;
		ASSERT_TRUE(mmapIntString);
	}

	{
		const bool mapIntIssr = IsStringRepresentable<std::map<int, StringRepresentableType>>::Value;
		ASSERT_TRUE(mapIntIssr);
		const bool mmapIntIssr = IsStringRepresentable<std::multimap<int, StringRepresentableType>>::Value;
		ASSERT_TRUE(mmapIntIssr);
	}

	{
		const bool mapIssrInt = IsStringRepresentable<std::map<StringRepresentableType, int>>::Value;
		ASSERT_TRUE(mapIssrInt);
		const bool mmapIssrInt = IsStringRepresentable<std::multimap<StringRepresentableType, int>>::Value;
		ASSERT_TRUE(mmapIssrInt);
	}

	{
		const bool mapIntNotsr = IsStringRepresentable<std::map<int, NotStringRepresentableType>>::Value;
		ASSERT_FALSE(mapIntNotsr);
		const bool mmapIntNotsr = IsStringRepresentable<std::multimap<int, NotStringRepresentableType>>::Value;
		ASSERT_FALSE(mmapIntNotsr);
	}

	{
		const bool mapNotsrInt = IsStringRepresentable<std::map<NotStringRepresentableType, int>>::Value;
		ASSERT_FALSE(mapNotsrInt);
		const bool mmapNotsrInt = IsStringRepresentable<std::multimap<NotStringRepresentableType, int>>::Value;
		ASSERT_FALSE(mmapNotsrInt);
	}

	{
		const bool pairIntString = IsStringRepresentable<std::pair<int, std::string>>::Value;
		ASSERT_TRUE(pairIntString);

		const bool pairIntIssr = IsStringRepresentable<std::pair<int, StringRepresentableType>>::Value;
		ASSERT_TRUE(pairIntIssr);

		const bool pairIssrInt = IsStringRepresentable<std::pair<StringRepresentableType, int>>::Value;
		ASSERT_TRUE(pairIssrInt);

		const bool pairIntNotsr = IsStringRepresentable<std::pair<int, NotStringRepresentableType>>::Value;
		ASSERT_FALSE(pairIntNotsr);

		const bool pairNotsrInt = IsStringRepresentable<std::pair<NotStringRepresentableType, int>>::Value;
		ASSERT_FALSE(pairNotsrInt);
	}

	{
		const int seq1[] = { 1 };
		ASSERT_TRUE(IsStringRepresentable<decltype(ToRange(seq1))>::Value);
		ASSERT_TRUE(IsStringRepresentable<decltype(EnumerableFromStlIterators(std::begin(seq1), std::end(seq1)))>::Value);

		const StringRepresentableType seq2[] = { StringRepresentableType("1") };
		ASSERT_TRUE(IsStringRepresentable<decltype(ToRange(seq2))>::Value);
		ASSERT_TRUE(IsStringRepresentable<decltype(EnumerableFromStlIterators(std::begin(seq2), std::end(seq2)))>::Value);

		const NotStringRepresentableType seq3[] = { NotStringRepresentableType() };
		ASSERT_FALSE(IsStringRepresentable<decltype(ToRange(seq3))>::Value);
		ASSERT_FALSE(IsStringRepresentable<decltype(EnumerableFromStlIterators(std::begin(seq3), std::end(seq3)))>::Value);
	}

	ASSERT_TRUE(IsStringRepresentable<char>::Value);
	ASSERT_TRUE(IsStringRepresentable<short>::Value);
	ASSERT_TRUE(IsStringRepresentable<int>::Value);
	ASSERT_TRUE(IsStringRepresentable<long>::Value);

	ASSERT_TRUE(IsStringRepresentable<u8>::Value);
	ASSERT_TRUE(IsStringRepresentable<s8>::Value);
	ASSERT_TRUE(IsStringRepresentable<u16>::Value);
	ASSERT_TRUE(IsStringRepresentable<s16>::Value);
	ASSERT_TRUE(IsStringRepresentable<u32>::Value);
	ASSERT_TRUE(IsStringRepresentable<s32>::Value);
	ASSERT_TRUE(IsStringRepresentable<u64>::Value);
	ASSERT_TRUE(IsStringRepresentable<s64>::Value);

	ASSERT_TRUE(IsStringRepresentable<bool>::Value);
	ASSERT_TRUE(IsStringRepresentable<std::string>::Value);
	ASSERT_TRUE(IsStringRepresentable<char*>::Value);
	ASSERT_TRUE(IsStringRepresentable<const char*>::Value);
	ASSERT_TRUE(IsStringRepresentable<void*>::Value);
	ASSERT_TRUE(IsStringRepresentable<const void*>::Value);

	ASSERT_TRUE(IsStringRepresentable<float>::Value);
	ASSERT_TRUE(IsStringRepresentable<double>::Value);
	ASSERT_TRUE(IsStringRepresentable<long double>::Value);

	ASSERT_TRUE(IsStringRepresentable<EmptyType>::Value);
	ASSERT_TRUE(IsStringRepresentable<NullPtrType>::Value);

	ASSERT_TRUE(IsStringRepresentable<shared_ptr<int>>::Value);
	ASSERT_TRUE(IsStringRepresentable<optional<int>>::Value);
	ASSERT_TRUE(IsStringRepresentable<optional<int>>::Value);

	ASSERT_TRUE(IsStringRepresentable<TestEnum>::Value);
	ASSERT_TRUE(IsStringRepresentable<TestException>::Value);

	ASSERT_TRUE(IsStringRepresentable<decltype(MakeTuple())>::Value);
	ASSERT_TRUE(IsStringRepresentable<decltype(MakeTuple(0, ""))>::Value);
	ASSERT_TRUE(IsStringRepresentable<decltype(MakeTuple(0, StringRepresentableType("1")))>::Value);
	ASSERT_FALSE(IsStringRepresentable<decltype(MakeTuple(0, NotStringRepresentableType()))>::Value);
}


TEST(ToStringTest, ToString)
{
	{
		const std::vector<int> vec0;
		const std::vector<int> vec1 = { 1 };
		const std::vector<int> vec2 = { 1, 2 };
		const std::vector<int> vec3 = { 1, 2, 1, 2 };
		const std::vector<int> vec4 = { 1, 2, 3, 4, 5 };

		ASSERT_EQ(ToString(vec0), "[]");
		ASSERT_EQ(ToString(vec1), "[1]");
		ASSERT_EQ(ToString(vec2), "[1, 2]");
		ASSERT_EQ(ToString(vec3), "[1, 2, 1, 2]");
		ASSERT_EQ(ToString(vec4), "[1, 2, 3, 4, 5]");
	}

	{
		const std::list<int> list0;
		const std::list<int> list1 = { 1 };
		const std::list<int> list2 = { 1, 2 };
		const std::list<int> list3 = { 1, 2, 1, 2 };
		const std::list<int> list4 = { 1, 2, 3, 4, 5 };

		ASSERT_EQ(ToString(list0), "[]");
		ASSERT_EQ(ToString(list1), "[1]");
		ASSERT_EQ(ToString(list2), "[1, 2]");
		ASSERT_EQ(ToString(list3), "[1, 2, 1, 2]");
		ASSERT_EQ(ToString(list4), "[1, 2, 3, 4, 5]");
	}

	{
		const std::deque<int> deq0;
		const std::deque<int> deq1 = { 1 };
		const std::deque<int> deq2 = { 1, 2 };
		const std::deque<int> deq3 = { 1, 2, 1, 2 };
		const std::deque<int> deq4 = { 1, 2, 3, 4, 5 };

		ASSERT_EQ(ToString(deq0), "[]");
		ASSERT_EQ(ToString(deq1), "[1]");
		ASSERT_EQ(ToString(deq2), "[1, 2]");
		ASSERT_EQ(ToString(deq3), "[1, 2, 1, 2]");
		ASSERT_EQ(ToString(deq4), "[1, 2, 3, 4, 5]");
	}

	{
		const std::set<int> set0;
		const std::set<int> set1 = { 1 };
		const std::set<int> set2 = { 1, 2 };
		const std::set<int> set3 = { 1, 2, 1, 2 };
		const std::set<int> set4 = { 1, 2, 3, 4, 5 };

		ASSERT_EQ(ToString(set0), "[]");
		ASSERT_EQ(ToString(set1), "[1]");
		ASSERT_EQ(ToString(set2), "[1, 2]");
		ASSERT_EQ(ToString(set3), "[1, 2]");
		ASSERT_EQ(ToString(set4), "[1, 2, 3, 4, 5]");
	}

	{
		const std::multiset<int> mset0;
		const std::multiset<int> mset1 = { 1 };
		const std::multiset<int> mset2 = { 1, 2 };
		const std::multiset<int> mset3 = { 1, 2, 1, 2 };
		const std::multiset<int> mset4 = { 1, 2, 3, 4, 5 };

		ASSERT_EQ(ToString(mset0), "[]");
		ASSERT_EQ(ToString(mset1), "[1]");
		ASSERT_EQ(ToString(mset2), "[1, 2]");
		ASSERT_EQ(ToString(mset3), "[1, 1, 2, 2]");
		ASSERT_EQ(ToString(mset4), "[1, 2, 3, 4, 5]");
	}

	{
		const std::map<int, std::string> map0;
		const std::map<int, std::string> map1 = { { 1, "1" } };
		const std::map<int, std::string> map2 = { { 1, "1" }, { 2, "2" } };
		const std::map<int, std::string> map3 = { { 1, "1" }, { 2, "2" }, { 1, "1" }, { 2, "2" } };
		const std::map<int, std::string> map4 = { { 1, "1" }, { 2, "2" }, { 3, "3" }, { 4, "4" }, { 5, "5" } };

		ASSERT_EQ(ToString(map0), "{ }");
		ASSERT_EQ(ToString(map1), "{ 1: 1 }");
		ASSERT_EQ(ToString(map2), "{ 1: 1, 2: 2 }");
		ASSERT_EQ(ToString(map3), "{ 1: 1, 2: 2 }");
		ASSERT_EQ(ToString(map4), "{ 1: 1, 2: 2, 3: 3, 4: 4, 5: 5 }");
	}

	{
		const std::multimap<int, std::string> mmap0;
		const std::multimap<int, std::string> mmap1 = { { 1, "1" } };
		const std::multimap<int, std::string> mmap2 = { { 1, "1" }, { 2, "2" } };
		const std::multimap<int, std::string> mmap3 = { { 1, "1" }, { 1, "1" }, { 2, "2" }, { 2, "2" } };
		const std::multimap<int, std::string> mmap4 = { { 1, "1" }, { 2, "2" }, { 3, "3" }, { 4, "4" }, { 5, "5" } };

		ASSERT_EQ(ToString(mmap0), "{ }");
		ASSERT_EQ(ToString(mmap1), "{ 1: 1 }");
		ASSERT_EQ(ToString(mmap2), "{ 1: 1, 2: 2 }");
		ASSERT_EQ(ToString(mmap3), "{ 1: 1, 1: 1, 2: 2, 2: 2 }");
		ASSERT_EQ(ToString(mmap4), "{ 1: 1, 2: 2, 3: 3, 4: 4, 5: 5 }");
	}

	{
		const int seq0[] = { 0 };
		const auto en0 = EnumerableFromStlIterators(std::next(std::begin(seq0), 1), std::end(seq0));

		const int seq1[] = { 1 };
		const auto en1 = EnumerableFromStlIterators(std::begin(seq1), std::end(seq1));

		const int seq2[] = { 1, 2 };
		const auto en2 = EnumerableFromStlIterators(std::begin(seq2), std::end(seq2));

		const int seq3[] = { 1, 2, 1, 2 };
		const auto en3 = EnumerableFromStlIterators(std::begin(seq3), std::end(seq3));

		const int seq4[] = { 1, 2, 3, 4, 5 };
		const auto en4 = EnumerableFromStlIterators(std::begin(seq4), std::end(seq4));

		ASSERT_EQ(ToString(ToRange(std::next(std::begin(seq0), 1), std::end(seq0))), "[]");
		ASSERT_EQ(ToString(en0), "[]");

		ASSERT_EQ(ToString(ToRange(seq1)), "[1]");
		ASSERT_EQ(ToString(en1), "[1]");

		ASSERT_EQ(ToString(ToRange(seq2)), "[1, 2]");
		ASSERT_EQ(ToString(en2), "[1, 2]");

		ASSERT_EQ(ToString(ToRange(seq3)), "[1, 2, 1, 2]");
		ASSERT_EQ(ToString(en3), "[1, 2, 1, 2]");

		ASSERT_EQ(ToString(ToRange(seq4)), "[1, 2, 3, 4, 5]");
		ASSERT_EQ(ToString(en4), "[1, 2, 3, 4, 5]");
	}

	{
		ASSERT_EQ(ToString((u8)0u), "0");
		ASSERT_EQ(ToString((u8)1u), "1");
		ASSERT_EQ(ToString((u8)12u), "12");
		ASSERT_EQ(ToString((u8)123u), "123");
		ASSERT_EQ(ToString((u8)254u), "254");
		ASSERT_EQ(ToString((u8)255u), "255");

		ASSERT_EQ(ToString((s8)-128), "-128");
		ASSERT_EQ(ToString((s8)-127), "-127");
		ASSERT_EQ(ToString((s8)-12), "-12");
		ASSERT_EQ(ToString((s8)-1), "-1");
		ASSERT_EQ(ToString((s8)0), "0");
		ASSERT_EQ(ToString((s8)12), "12");
		ASSERT_EQ(ToString((s8)126), "126");
		ASSERT_EQ(ToString((s8)127), "127");
	}

	{
		ASSERT_EQ(ToString((u16)0u), "0");
		ASSERT_EQ(ToString((u16)1u), "1");
		ASSERT_EQ(ToString((u16)12u), "12");
		ASSERT_EQ(ToString((u16)123u), "123");
		ASSERT_EQ(ToString((u16)65534u), "65534");
		ASSERT_EQ(ToString((u16)65535u), "65535");

		ASSERT_EQ(ToString((s16)-32768), "-32768");
		ASSERT_EQ(ToString((s16)-32767), "-32767");
		ASSERT_EQ(ToString((s16)-123), "-123");
		ASSERT_EQ(ToString((s16)-12), "-12");
		ASSERT_EQ(ToString((s16)-1), "-1");
		ASSERT_EQ(ToString((s16)0), "0");
		ASSERT_EQ(ToString((s16)12), "12");
		ASSERT_EQ(ToString((s16)123), "123");
		ASSERT_EQ(ToString((s16)32766), "32766");
		ASSERT_EQ(ToString((s16)32767), "32767");
	}

	{
		ASSERT_EQ(ToString((u32)0u), "0");
		ASSERT_EQ(ToString((u32)1u), "1");
		ASSERT_EQ(ToString((u32)12u), "12");
		ASSERT_EQ(ToString((u32)123u), "123");
		ASSERT_EQ(ToString((u32)4294967294u), "4294967294");
		ASSERT_EQ(ToString((u32)4294967295u), "4294967295");

		ASSERT_EQ(ToString((s32)-2147483648), "-2147483648");
		ASSERT_EQ(ToString((s32)-2147483647), "-2147483647");
		ASSERT_EQ(ToString((s32)-123), "-123");
		ASSERT_EQ(ToString((s32)-12), "-12");
		ASSERT_EQ(ToString((s32)-1), "-1");
		ASSERT_EQ(ToString((s32)0), "0");
		ASSERT_EQ(ToString((s32)12), "12");
		ASSERT_EQ(ToString((s32)123), "123");
		ASSERT_EQ(ToString((s32)2147483646), "2147483646");
		ASSERT_EQ(ToString((s32)2147483647), "2147483647");
	}

	{
		ASSERT_EQ(ToString((u64)0u), "0");
		ASSERT_EQ(ToString((u64)1u), "1");
		ASSERT_EQ(ToString((u64)12u), "12");
		ASSERT_EQ(ToString((u64)123u), "123");
		ASSERT_EQ(ToString((u64)18446744073709551614ull), "18446744073709551614");
		ASSERT_EQ(ToString((u64)18446744073709551615ull), "18446744073709551615");

		ASSERT_EQ(ToString((s64)(-9223372036854775807ll - 1)), "-9223372036854775808");
		ASSERT_EQ(ToString((s64)-9223372036854775807ll), "-9223372036854775807");
		ASSERT_EQ(ToString((s64)-123), "-123");
		ASSERT_EQ(ToString((s64)-12), "-12");
		ASSERT_EQ(ToString((s64)-1), "-1");
		ASSERT_EQ(ToString((s64)0), "0");
		ASSERT_EQ(ToString((s64)12), "12");
		ASSERT_EQ(ToString((s64)123), "123");
		ASSERT_EQ(ToString((s64)9223372036854775806ll), "9223372036854775806");
		ASSERT_EQ(ToString((s64)9223372036854775807ll), "9223372036854775807");
	}

	{
		ASSERT_EQ(ToString(true), "true");
		ASSERT_EQ(ToString(false), "false");

		{
			ASSERT_EQ(ToString(std::string()), "");
			ASSERT_EQ(ToString(std::string("str1")), "str1");

			ASSERT_EQ(ToString(string_view()), "");
			ASSERT_EQ(ToString(string_view("str2")), "str2");

			const char* str2 = "";
			ASSERT_EQ(ToString(str2), "");
			const char* str3 = "str3";
			ASSERT_EQ(ToString(str3), "str3");
			char str4[] = "";
			ASSERT_EQ(ToString(static_cast<char*>(str4)), "");
			char str5[] = "str5";
			ASSERT_EQ(ToString(static_cast<char*>(str5)), "str5");
		}

		ASSERT_EQ(ToString('-'), "-");
		ASSERT_EQ(ToString('!'), "!");
		ASSERT_EQ(ToString('@'), "@");
		ASSERT_EQ(ToString('#'), "#");
		ASSERT_EQ(ToString('0'), "0");
		ASSERT_EQ(ToString('1'), "1");
		ASSERT_EQ(ToString('9'), "9");
		ASSERT_EQ(ToString('a'), "a");
		ASSERT_EQ(ToString('z'), "z");
		ASSERT_EQ(ToString('A'), "A");
		ASSERT_EQ(ToString('Z'), "Z");

		ASSERT_EQ(ToString('\0'), std::string(1, 0));
		ASSERT_EQ(ToString('\1'), "\1");
		ASSERT_EQ(ToString('\r'), "\r");
		ASSERT_EQ(ToString('\n'), "\n");
		ASSERT_EQ(ToString('\376'), std::string(1, (char)254));
		ASSERT_EQ(ToString('\377'), std::string(1, (char)255));

		ASSERT_EQ(ToString(static_cast<void*>(0)), "(nil)");
		ASSERT_EQ(ToString(static_cast<const void*>(0)), "(nil)");
		ASSERT_EQ(ToString(reinterpret_cast<void*>(0x12)), "0x12");
		ASSERT_EQ(ToString(reinterpret_cast<const void*>(0x12)), "0x12");
		ASSERT_EQ(ToString(reinterpret_cast<void*>(0x1234)), "0x1234");
		ASSERT_EQ(ToString(reinterpret_cast<const void*>(0x1234)), "0x1234");
		ASSERT_EQ(ToString(reinterpret_cast<void*>(0x123456)), "0x123456");
		ASSERT_EQ(ToString(reinterpret_cast<const void*>(0x123456)), "0x123456");
		ASSERT_EQ(ToString(reinterpret_cast<void*>(0x12345678)), "0x12345678");
		ASSERT_EQ(ToString(reinterpret_cast<const void*>(0x12345678)), "0x12345678");

		ASSERT_EQ(ToString(-0.12345679f), "-0.1234568");
		ASSERT_EQ(ToString(-0.1234567f), "-0.1234567");
		ASSERT_EQ(ToString(-0.123f), "-0.123");
		ASSERT_EQ(ToString(-0.12f), "-0.12");
		ASSERT_EQ(ToString(-0.1f), "-0.1");
		ASSERT_EQ(ToString(-0.0f), "-0");
		ASSERT_EQ(ToString(0.0f), "0");
		ASSERT_EQ(ToString(0.1f), "0.1");
		ASSERT_EQ(ToString(0.12f), "0.12");
		ASSERT_EQ(ToString(0.123f), "0.123");
		ASSERT_EQ(ToString(0.1234567f), "0.1234567");
		ASSERT_EQ(ToString(0.12345679f), "0.1234568");

		ASSERT_EQ(ToString(-0.12345678901234569), "-0.1234567890123457");
		ASSERT_EQ(ToString(-0.1234567890123456), "-0.1234567890123456");
		ASSERT_EQ(ToString(-0.123), "-0.123");
		ASSERT_EQ(ToString(-0.12), "-0.12");
		ASSERT_EQ(ToString(-0.1), "-0.1");
		ASSERT_EQ(ToString(-0.0), "-0");
		ASSERT_EQ(ToString(0.0), "0");
		ASSERT_EQ(ToString(0.1), "0.1");
		ASSERT_EQ(ToString(0.12), "0.12");
		ASSERT_EQ(ToString(0.123), "0.123");
		ASSERT_EQ(ToString(0.1234567890123456), "0.1234567890123456");
		ASSERT_EQ(ToString(0.12345678901234569), "0.1234567890123457");

		ASSERT_EQ(ToString(-0.12345678901234569l), "-0.1234567890123457");
		ASSERT_EQ(ToString(-0.1234567890123456l), "-0.1234567890123456");
		ASSERT_EQ(ToString(-0.123l), "-0.123");
		ASSERT_EQ(ToString(-0.12l), "-0.12");
		ASSERT_EQ(ToString(-0.1l), "-0.1");
		ASSERT_EQ(ToString(-0.0l), "-0");
		ASSERT_EQ(ToString(0.0l), "0");
		ASSERT_EQ(ToString(0.1l), "0.1");
		ASSERT_EQ(ToString(0.12l), "0.12");
		ASSERT_EQ(ToString(0.123l), "0.123");
		ASSERT_EQ(ToString(0.1234567890123456l), "0.1234567890123456");
		ASSERT_EQ(ToString(0.12345678901234569l), "0.1234567890123457");
	}

	{
		ASSERT_EQ(ToString(EmptyType()), "");
		ASSERT_EQ(ToString(null), "null");

		ASSERT_EQ(ToString(shared_ptr<int>()), "null");
		ASSERT_EQ(ToString(make_shared_ptr<int>(0)), "0");
		ASSERT_EQ(ToString(make_shared_ptr<int>(1)), "1");

		ASSERT_EQ(ToString(optional<int>()), "null");
		ASSERT_EQ(ToString(make_optional_value(0)), "0");
		ASSERT_EQ(ToString(make_optional_value(1)), "1");

		ASSERT_EQ(ToString(std::make_pair(0, "")), "(0, )");
		ASSERT_EQ(ToString(std::make_pair(1, "abc")), "(1, abc)");

		const std::string exStr = ToString(TestException("a1b2c3"));
		ASSERT_EQ(exStr.substr(exStr.find("TestException")), "TestException\na1b2c3");

		ASSERT_EQ(ToString(StringRepresentableType("")), "StringRepresentableType {  }");
		ASSERT_EQ(ToString(StringRepresentableType("1")), "StringRepresentableType { 1 }");
		ASSERT_EQ(ToString(StringRepresentableType("abc")), "StringRepresentableType { abc }");

		ASSERT_EQ(ToString(TestEnum(TestEnum::Test)), "Test");
		ASSERT_EQ(ToString(TestEnum(TestEnum::Enum_)), "Enum_");
		ASSERT_EQ(ToString(TestEnum(TestEnum::Value)), "Value");
		ASSERT_EQ(ToString(TestEnum::Test), "1");
		ASSERT_EQ(ToString(TestEnum::Enum_), "2");
		ASSERT_EQ(ToString(TestEnum::Value), "3");

		ASSERT_EQ(ToString(ArgileEnum(ArgileEnum::None)), "4");
		ASSERT_EQ(ToString(ArgileEnum(ArgileEnum::Value1)), "5");
		ASSERT_EQ(ToString(ArgileEnum(ArgileEnum::Value2)), "6");
		ASSERT_EQ(ToString(ArgileEnum::None), "4");
		ASSERT_EQ(ToString(ArgileEnum::Value1), "5");
		ASSERT_EQ(ToString(ArgileEnum::Value2), "6");

		ASSERT_EQ(ToString(MakeTuple()), "()");
		ASSERT_EQ(ToString(MakeTuple(0)), "(0)");
		ASSERT_EQ(ToString(MakeTuple(1)), "(1)");
		ASSERT_EQ(ToString(MakeTuple(1, "abc")), "(1, abc)");
		ASSERT_EQ(ToString(MakeTuple(0, "", 0.0)), "(0, , 0)");
		ASSERT_EQ(ToString(MakeTuple(1, "abc", 1.2)), "(1, abc, 1.2)");
	}
}

TEST(ToStringTest, StringBuilder)
{
	ASSERT_EQ(StringBuilder().ToString(), "");
	ASSERT_EQ((StringBuilder() % "").ToString(), "");
	ASSERT_EQ((StringBuilder() % "1" % 2 % "3" % "4").ToString(), "1234");

	{
		StringBuilder sb;
		ASSERT_TRUE(sb.empty());

		sb % 1;
		ASSERT_FALSE(sb.empty());
		ASSERT_EQ(sb.ToString(), "1");

		sb % "2";
		ASSERT_FALSE(sb.empty());
		ASSERT_EQ(sb.ToString(), "12");
	}
}

TEST(ToStringTest, StringJoiner)
{
	{
		ASSERT_EQ(StringJoiner("").ToString(), "");
		ASSERT_EQ((StringJoiner("") % "").ToString(), "");
		ASSERT_EQ((StringJoiner("") % "1" % 2 % "3" % "4").ToString(), "1234");

		{
			StringJoiner sj("");
			ASSERT_TRUE(sj.empty());

			sj % 1;
			ASSERT_FALSE(sj.empty());
			ASSERT_EQ(sj.ToString(), "1");

			sj % "2";
			ASSERT_FALSE(sj.empty());
			ASSERT_EQ(sj.ToString(), "12");
		}
	}

	{
		ASSERT_EQ(StringJoiner(", ").ToString(), "");
		ASSERT_EQ((StringJoiner(", ") % "").ToString(), "");
		ASSERT_EQ((StringJoiner(", ") % "1" % 2 % "3" % "4").ToString(), "1, 2, 3, 4");

		{
			StringJoiner sj(", ");
			ASSERT_TRUE(sj.empty());

			sj % 1;
			ASSERT_FALSE(sj.empty());
			ASSERT_EQ(sj.ToString(), "1");

			sj % "2";
			ASSERT_FALSE(sj.empty());
			ASSERT_EQ(sj.ToString(), "1, 2");
		}
	}

	{
		ASSERT_EQ(StringJoiner(", ", "{ ", " }").ToString(), "{  }");
		ASSERT_EQ((StringJoiner(", ", "{ ", " }") % "").ToString(), "{  }");
		ASSERT_EQ((StringJoiner(", ", "{ ", " }") % "1" % 2 % "3" % "4").ToString(), "{ 1, 2, 3, 4 }");

		{
			StringJoiner sj(", ", "{ ", " }");
			ASSERT_TRUE(sj.empty());
			ASSERT_EQ(sj.ToString(), "{  }");

			sj % 1;
			ASSERT_FALSE(sj.empty());
			ASSERT_EQ(sj.ToString(), "{ 1 }");

			sj % "2";
			ASSERT_FALSE(sj.empty());
			ASSERT_EQ(sj.ToString(), "{ 1, 2 }");
		}
	}
}
