#include <stingraykit/metaprogramming/NestedTypeCheck.h>

#include <gtest/gtest.h>

using namespace stingray;

namespace
{

	STINGRAYKIT_DECLARE_NESTED_TYPE_CHECK(Nested);

	struct HasNestedTypeNamedNested			{ struct Nested {}; };
	struct HasTypedefNamedNested			{ typedef int Nested; };
	struct HasReferenceTypedefNamedNested	{ typedef int& Nested; };
	struct HasFieldNamedNested				{ int Nested; };
	struct HasMethodNamedNested				{ void Nested() { } };

}

TEST(HasNestedTypeTest, NestedType)
{ ASSERT_TRUE(HasNestedType_Nested<HasNestedTypeNamedNested>::Value); }

TEST(HasNestedTypeTest, Typedef)
{ ASSERT_TRUE(HasNestedType_Nested<HasTypedefNamedNested>::Value); }

TEST(HasNestedTypeTest, ReferenceTypedef)
{ ASSERT_TRUE(HasNestedType_Nested<HasReferenceTypedefNamedNested>::Value); }

TEST(HasNestedTypeTest, Field)
{ ASSERT_TRUE(!HasNestedType_Nested<HasFieldNamedNested>::Value); }

TEST(HasNestedTypeTest, Method)
{ ASSERT_TRUE(!HasNestedType_Nested<HasMethodNamedNested>::Value); }

TEST(HasNestedTypeTest, NonClass)
{ ASSERT_TRUE(!HasNestedType_Nested<int>::Value); }
