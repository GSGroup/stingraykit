#include <stingraykit/Version.h>

#include <gtest/gtest.h>

using namespace stingray;

TEST(VersionTest, FromString)
{
	ASSERT_EQ(Version(1, 2), Version::FromString("1.2"));
	ASSERT_EQ(Version(1, 2, 3), Version::FromString("1.2.3"));

	ASSERT_ANY_THROW(Version::FromString("1."));
	ASSERT_ANY_THROW(Version::FromString("1.2."));

	ASSERT_ANY_THROW(Version::FromString("1azaz.2"));
	ASSERT_ANY_THROW(Version::FromString("1.2azaz.3"));
	ASSERT_ANY_THROW(Version::FromString("1.2.3azaz"));
}


TEST(VersionTest, ToString)
{
	Version v1(1, 2, 3);
	ASSERT_EQ(v1.ToString(), "1.2.3");

	Version v2(1, 2);
	ASSERT_EQ(v2.ToString(), "1.2");
}


TEST(VersionTest, Compare)
{
	Version v1(1, 2);
	Version v2(1, 2, 3);
	Version v3(1, 3);

	ASSERT_EQ(v1, v1);
	ASSERT_EQ(v2, v2);
	ASSERT_EQ(v3, v3);

	ASSERT_NE(v1, v2);
	ASSERT_NE(v2, v3);
	ASSERT_NE(v1, v3);

	ASSERT_LT(v1, v2);
	ASSERT_LT(v2, v3);
	ASSERT_LT(v1, v3);
}
