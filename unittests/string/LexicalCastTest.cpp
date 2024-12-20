// Copyright (c) 2011 - 2024, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/string/lexical_cast.h>

#include <gtest/gtest.h>

using namespace stingray;

namespace
{

	struct TestEnum1
	{
		STINGRAYKIT_ENUM_VALUES
		(
			Value1,
			Value2
		);

		STINGRAYKIT_DECLARE_ENUM_CLASS(TestEnum1);
	};

	struct TestEnum2
	{
		STINGRAYKIT_ENUM_VALUES
		(
			Value1,
			Value2,
			Value3
		);

		STINGRAYKIT_DECLARE_ENUM_CLASS(TestEnum2);
	};

}

TEST(LexicalCastTest, LexicalCaster)
{
	{
		const TestEnum2 val = lexical_caster(TestEnum1(TestEnum1::Value1));
		ASSERT_EQ(val, TestEnum2(TestEnum2::Value1));
	}

	{
		const TestEnum2 val = lexical_caster(TestEnum1(TestEnum1::Value2));
		ASSERT_EQ(val, TestEnum2(TestEnum2::Value2));
	}

	{
		const TestEnum1 val = lexical_caster(TestEnum2(TestEnum2::Value1));
		ASSERT_EQ(val, TestEnum1(TestEnum1::Value1));
	}

	{
		const TestEnum1 val = lexical_caster(TestEnum2(TestEnum2::Value2));
		ASSERT_EQ(val, TestEnum1(TestEnum1::Value2));
	}

	{
		TestEnum1 val;
		ASSERT_ANY_THROW(val = lexical_caster(TestEnum2(TestEnum2::Value3)));
	}

	{
		const std::string val = lexical_caster(12345);
		ASSERT_EQ(val, "12345");
	}

	{
		const int val = lexical_caster(12345);
		ASSERT_EQ(val, 12345);
	}

	{
		const std::string val = lexical_caster(std::string("12345"));
		ASSERT_EQ(val, "12345");
	}

	{
		const int val = lexical_caster(std::string("12345"));
		ASSERT_EQ(val, 12345);
	}
}
