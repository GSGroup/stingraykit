#include <stingraykit/string/string_view.h>

#include <gtest/gtest.h>

using namespace stingray;

namespace
{

	const char* sample = "two girls one cup";
	const string_view::size_type sampleSize = ::strlen(sample);

	const char* sampleWithNull = "two girls\0 one cup";
	const string_view::size_type sampleWithNullSize = ::strlen(sampleWithNull);

	const std::string sampleStr = "two girls one cup";

	const char* sample2 = "one girl two cups";
	const string_view::size_type sample2Size = ::strlen(sample2);

	const char* sample3 = "two girls two cup";
	const string_view::size_type sample3Size = ::strlen(sample);

	bool checkStr(const char* testee, string_view::size_type len, string_view sample)
	{
		for (string_view::size_type i = 0; i < len && i < sample.size(); ++i)
			if (testee[i] != sample[i])
				return false;
		return true;
	}

}


TEST(StringViewTest, Construction)
{
	{
		string_view str;
		EXPECT_EQ(str.size(), (string_view::size_type)0);
		EXPECT_TRUE(str.empty());
	}
	{
		string_view str((string_view()));
		EXPECT_EQ(str.size(), (string_view::size_type)0);
		EXPECT_TRUE(str.empty());
	}
	{
		string_view str(sample);
		EXPECT_EQ(str.data(), sample);
		EXPECT_EQ(str.size(), sampleSize);
		EXPECT_FALSE(str.empty());
	}
	{
		string_view str(sampleWithNull);
		EXPECT_EQ(str.data(), sampleWithNull);
		EXPECT_EQ(str.size(), sampleWithNullSize);
		EXPECT_FALSE(str.empty());
	}
	{
		string_view str(sample, 4);
		EXPECT_EQ(str.data(), sample);
		EXPECT_EQ(str.size(), (string_view::size_type)4);
		EXPECT_FALSE(str.empty());
	}
	{
		string_view str(sampleWithNull, sampleSize + 1);
		EXPECT_EQ(str.data(), sampleWithNull);
		EXPECT_EQ(str.size(), sampleSize + 1);
		EXPECT_FALSE(str.empty());
	}
	{
		string_view str(sampleStr);
		EXPECT_EQ(str.data(), sampleStr.data());
		EXPECT_EQ(str.size(), sampleStr.size());
		EXPECT_FALSE(str.empty());
	}
}


TEST(StringViewTest, Assign)
{
	{
		string_view str;
		str = sample;
		EXPECT_EQ(str.data(), sample);
		EXPECT_EQ(str.size(), sampleSize);
		EXPECT_FALSE(str.empty());
	}
	{
		string_view str;
		str = sampleWithNull;
		EXPECT_EQ(str.data(), sampleWithNull);
		EXPECT_EQ(str.size(), sampleWithNullSize);
		EXPECT_FALSE(str.empty());
	}
	{
		string_view str;
		str = string_view(sample, 4);
		EXPECT_EQ(str.data(), sample);
		EXPECT_EQ(str.size(), (string_view::size_type)4);
		EXPECT_FALSE(str.empty());
	}
	{
		string_view str;
		str = string_view(sampleWithNull, sampleSize + 1);
		EXPECT_EQ(str.data(), sampleWithNull);
		EXPECT_EQ(str.size(), sampleSize + 1);
		EXPECT_FALSE(str.empty());
	}
	{
		string_view str;
		str = sampleStr;
		EXPECT_EQ(str.data(), sampleStr.data());
		EXPECT_EQ(str.size(), sampleStr.size());
		EXPECT_FALSE(str.empty());
	}
}


TEST(StringViewTest, Iterators)
{
	{
		string_view str;
		EXPECT_EQ(str.begin(), str.end());
		EXPECT_EQ(str.rbegin(), str.rend());
	}
	{
		string_view str(sample);
		EXPECT_NE(str.begin(), str.end());
		EXPECT_EQ(*str.begin(), *sample);
		EXPECT_EQ(*(str.end() - 1), sample[sampleSize - 1]);
		EXPECT_EQ(*str.rbegin(), sample[sampleSize - 1]);
		EXPECT_EQ(*(--str.rend()), *sample);
	}
}


TEST(StringViewTest, Accessors)
{
	string_view str(sample);
	ASSERT_NO_THROW(str.front());
	ASSERT_NO_THROW(str.back());
	ASSERT_NO_THROW(str[0]);
	ASSERT_NO_THROW(str.at(0));
	ASSERT_ANY_THROW(str.at(99));

	ASSERT_ANY_THROW(string_view().at(0));
}


TEST(StringViewTest, Modifiers)
{
	{
		string_view base(sample);
		string_view str(sample);
		str.remove_prefix(0);
		EXPECT_EQ(str.data(), base.data());
		str.remove_prefix(3);
		EXPECT_EQ(str.data(), base.data() + 3);
		EXPECT_ANY_THROW(str.remove_prefix(99));
	}
	{
		string_view base(sample);
		string_view str(sample);
		str.remove_suffix(0);
		EXPECT_EQ(str.size(), base.size());
		str.remove_suffix(3);
		EXPECT_EQ(str.size(), base.size() - 3);
		EXPECT_ANY_THROW(str.remove_suffix(99));
	}
	{
		string_view base(sample);
		string_view str(sample);
		str.remove_suffix(3);
		EXPECT_EQ(str.size(), base.size() - 3);
		base.swap(str);
		EXPECT_EQ(base.size(), str.size() - 3);
		std::swap(base, str);
		EXPECT_EQ(str.size(), base.size() - 3);
	}
	{
		string_view base(sample);
		base.remove_prefix(base.size());
		EXPECT_EQ(base.size(), (string_view::size_type)0);
	}
	{
		string_view base(sample);
		base.remove_suffix(base.size());
		EXPECT_EQ(base.size(), (string_view::size_type)0);
	}
}


TEST(StringViewTest, OperationsCopy)
{
	string_view str(sample);
	std::string res = str.copy();
	EXPECT_EQ(str, res);
}


TEST(StringViewTest, OperationsSubstr)
{
	EXPECT_EQ(string_view(sample).substr(10), string_view(sample + 10));
	EXPECT_EQ(string_view(sample).substr(10, 3), string_view(sample + 10, 3));
	EXPECT_EQ(string_view(sample).substr(10, 0), string_view());
	EXPECT_EQ(string_view(sample).substr(sampleSize), string_view());
	ASSERT_ANY_THROW(string_view(sample).substr(sampleSize + 1));
}


TEST(StringViewTest, OperationsCompare)
{
	EXPECT_EQ(string_view(sample).compare(string_view(sample)), 0);
	EXPECT_EQ(string_view(sample).compare(3, sampleSize - 3, string_view(sample + 3)), 0);
	EXPECT_EQ(string_view(sample).compare(3, sampleSize - 3, string_view(sample), 3, sampleSize - 3), 0);

	EXPECT_GT(string_view(sample).compare(string_view(sample2)), 0);
	EXPECT_LT(string_view(sample2).compare(string_view(sample)), 0);
	EXPECT_EQ(string_view(sample).compare(0, 3, string_view(sample2), 9, 3), 0);

	EXPECT_TRUE(string_view(sample) == sample);
	EXPECT_TRUE(string_view(sample).substr(0, 3) == "two");
}


TEST(StringViewTest, OperationsStartsEndsWith)
{
	EXPECT_TRUE(string_view("abc").starts_with("abc"));
	EXPECT_TRUE(string_view("abc").starts_with("ab"));
	EXPECT_TRUE(string_view("abc").starts_with("a"));

	EXPECT_FALSE(string_view("abc").starts_with("abcd"));
	EXPECT_FALSE(string_view("abc").starts_with("bc"));
	EXPECT_FALSE(string_view("abc").starts_with("c"));

	EXPECT_FALSE(string_view("abc").starts_with("ABC"));

	EXPECT_TRUE(string_view("abc").ends_with("abc"));
	EXPECT_TRUE(string_view("abc").ends_with("bc"));
	EXPECT_TRUE(string_view("abc").ends_with("c"));

	EXPECT_FALSE(string_view("abc").ends_with("abcd"));
	EXPECT_FALSE(string_view("abc").ends_with("ab"));
	EXPECT_FALSE(string_view("abc").ends_with("a"));

	EXPECT_FALSE(string_view("abc").ends_with("ABC"));
}


TEST(StringViewTest, OperationsFind)
{
	EXPECT_EQ(string_view(sample).find(string_view()), (string_view::size_type)0);
	EXPECT_EQ(string_view(sample).find(string_view(), 4), (string_view::size_type)4);

	EXPECT_EQ(string_view(sample).find("two"), (string_view::size_type)0);
	EXPECT_EQ(string_view(sample).find("girls"), (string_view::size_type)4);
	EXPECT_EQ(string_view(sample).find("one"), (string_view::size_type)10);
	EXPECT_EQ(string_view(sample).find("cup"), (string_view::size_type)14);

	EXPECT_EQ(string_view(sample3).find("two"), (string_view::size_type)0);
	EXPECT_EQ(string_view(sample3).find("two", 1), (string_view::size_type)10);

	EXPECT_EQ(string_view(sample + 3).find("two"), (string_view::size_type)string_view::npos);
	EXPECT_EQ(string_view(sample).find("two", 3), (string_view::size_type)string_view::npos);

	EXPECT_EQ(string_view(sampleWithNull, sampleSize + 1).find("two"), (string_view::size_type)0);
	EXPECT_EQ(string_view(sampleWithNull, sampleSize + 1).find("girls"), (string_view::size_type)4);
	EXPECT_EQ(string_view(sampleWithNull, sampleSize + 1).find("one"), (string_view::size_type)11);
	EXPECT_EQ(string_view(sampleWithNull, sampleSize + 1).find("cup"), (string_view::size_type)15);
}


TEST(StringViewTest, OperationsRfind)
{
	EXPECT_EQ(string_view(sample).rfind(string_view()), (string_view::size_type)sampleSize);
	EXPECT_EQ(string_view(sample).rfind(string_view(), 4), (string_view::size_type)4);

	EXPECT_EQ(string_view(sample).rfind("two"), (string_view::size_type)0);
	EXPECT_EQ(string_view(sample).rfind("girls"), (string_view::size_type)4);
	EXPECT_EQ(string_view(sample).rfind("one"), (string_view::size_type)10);
	EXPECT_EQ(string_view(sample).rfind("cup"), (string_view::size_type)14);

	EXPECT_EQ(string_view(sample3).rfind("two"), (string_view::size_type)10);
	EXPECT_EQ(string_view(sample3).rfind("two", 9), (string_view::size_type)0);

	EXPECT_EQ(string_view(sample + 3).rfind("two"), (string_view::size_type)string_view::npos);
	EXPECT_EQ(string_view(sample).rfind("two", 3), (string_view::size_type)0);

	EXPECT_EQ(string_view(sampleWithNull, sampleSize + 1).rfind("two"), (string_view::size_type)0);
	EXPECT_EQ(string_view(sampleWithNull, sampleSize + 1).rfind("girls"), (string_view::size_type)4);
	EXPECT_EQ(string_view(sampleWithNull, sampleSize + 1).rfind("one"), (string_view::size_type)11);
	EXPECT_EQ(string_view(sampleWithNull, sampleSize + 1).rfind("cup"), (string_view::size_type)15);
}


TEST(StringViewTest, OperationsFindOf)
{
	EXPECT_EQ(string_view(sample).find_first_of("abc"), (string_view::size_type)14);
	EXPECT_EQ(string_view(sample).find_first_of("ab"), (string_view::size_type)string_view::npos);

	EXPECT_EQ(string_view(sample).find_last_of("abc"), (string_view::size_type)14);
	EXPECT_EQ(string_view(sample).find_last_of("ab"), (string_view::size_type)string_view::npos);
	EXPECT_EQ(string_view(sample).find_last_of("t"), (string_view::size_type)0);

	EXPECT_EQ(string_view(sample).find_first_not_of("abc"), (string_view::size_type)0);
	EXPECT_EQ(string_view(sample).find_first_not_of("abt"), (string_view::size_type)1);

	EXPECT_EQ(string_view(sample).find_last_not_of("abc"), (string_view::size_type)sampleSize - 1);
	EXPECT_EQ(string_view(sample).find_last_not_of("abt"), (string_view::size_type)sampleSize - 1);
	EXPECT_EQ(string_view(sample).find_last_of("t"), (string_view::size_type)0);
}
