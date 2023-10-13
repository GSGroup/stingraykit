#include <stingraykit/string/Utf8StringUtils.h>

#include <gmock/gmock-matchers.h>

using namespace stingray;

TEST(Utf8IteratorRangeTest, Next)
{
	{
		const std::string str;
		Utf8IteratorRange<std::string> range(str);

		ASSERT_ANY_THROW(range.Get());
		ASSERT_ANY_THROW(range.Next());
	}

	{
		const std::string str("TestString");
		Utf8IteratorRange<std::string> range(str);

		ASSERT_EQ(range.Get(), 'T');
		ASSERT_NO_THROW(range.Next());

		ASSERT_EQ(range.Get(), 'e');
		ASSERT_NO_THROW(range.Next());

		ASSERT_EQ(range.Get(), 's');
		ASSERT_NO_THROW(range.Next());

		ASSERT_EQ(range.Get(), 't');
		ASSERT_NO_THROW(range.Next());

		ASSERT_EQ(range.Get(), 'S');
		ASSERT_NO_THROW(range.Next());

		ASSERT_EQ(range.Get(), 't');
		ASSERT_NO_THROW(range.Next());

		ASSERT_EQ(range.Get(), 'r');
		ASSERT_NO_THROW(range.Next());

		ASSERT_EQ(range.Get(), 'i');
		ASSERT_NO_THROW(range.Next());

		ASSERT_EQ(range.Get(), 'n');
		ASSERT_NO_THROW(range.Next());

		ASSERT_EQ(range.Get(), 'g');
		ASSERT_NO_THROW(range.Next());

		ASSERT_ANY_THROW(range.Get());
		ASSERT_ANY_THROW(range.Next());
	}

	{
		const std::string str("TestСтрока");
		Utf8IteratorRange<std::string> range(str);

		ASSERT_EQ(range.Get(), 'T');
		ASSERT_NO_THROW(range.Next());

		ASSERT_EQ(range.Get(), 'e');
		ASSERT_NO_THROW(range.Next());

		ASSERT_EQ(range.Get(), 's');
		ASSERT_NO_THROW(range.Next());

		ASSERT_EQ(range.Get(), 't');
		ASSERT_NO_THROW(range.Next());

		ASSERT_EQ(range.Get(), L'С');
		ASSERT_NO_THROW(range.Next());

		ASSERT_EQ(range.Get(), L'т');
		ASSERT_NO_THROW(range.Next());

		ASSERT_EQ(range.Get(), L'р');
		ASSERT_NO_THROW(range.Next());

		ASSERT_EQ(range.Get(), L'о');
		ASSERT_NO_THROW(range.Next());

		ASSERT_EQ(range.Get(), L'к');
		ASSERT_NO_THROW(range.Next());

		ASSERT_EQ(range.Get(), L'а');
		ASSERT_NO_THROW(range.Next());

		ASSERT_ANY_THROW(range.Get());
		ASSERT_ANY_THROW(range.Next());
	}

	{
		const std::string str("ТестString");
		Utf8IteratorRange<std::string> range(str);

		ASSERT_EQ(range.Get(), L'Т');
		ASSERT_NO_THROW(range.Next());

		ASSERT_EQ(range.Get(), L'е');
		ASSERT_NO_THROW(range.Next());

		ASSERT_EQ(range.Get(), L'с');
		ASSERT_NO_THROW(range.Next());

		ASSERT_EQ(range.Get(), L'т');
		ASSERT_NO_THROW(range.Next());

		ASSERT_EQ(range.Get(), 'S');
		ASSERT_NO_THROW(range.Next());

		ASSERT_EQ(range.Get(), 't');
		ASSERT_NO_THROW(range.Next());

		ASSERT_EQ(range.Get(), 'r');
		ASSERT_NO_THROW(range.Next());

		ASSERT_EQ(range.Get(), 'i');
		ASSERT_NO_THROW(range.Next());

		ASSERT_EQ(range.Get(), 'n');
		ASSERT_NO_THROW(range.Next());

		ASSERT_EQ(range.Get(), 'g');
		ASSERT_NO_THROW(range.Next());

		ASSERT_ANY_THROW(range.Get());
		ASSERT_ANY_THROW(range.Next());
	}

	{
		const std::string str("\x92\xa5\xe1\xe2\x91\xe2\xe0\xae\xaa\xa0"); // ТестСтрока in CP866
		Utf8IteratorRange<std::string> range(str);

		ASSERT_ANY_THROW(range.Get());
		ASSERT_ANY_THROW(range.Next());
	}
}

TEST(Utf8IteratorRangeTest, Prev)
{
	{
		const std::string str;
		Utf8IteratorRange<std::string> range(str, str.end());

		ASSERT_ANY_THROW(range.Get());
		ASSERT_ANY_THROW(range.Prev());
	}

	{
		const std::string str("TestString");
		Utf8IteratorRange<std::string> range(str, str.end());

		ASSERT_ANY_THROW(range.Get());
		ASSERT_ANY_THROW(range.Next());

		ASSERT_NO_THROW(range.Prev());
		ASSERT_EQ(range.Get(), 'g');

		ASSERT_NO_THROW(range.Prev());
		ASSERT_EQ(range.Get(), 'n');

		ASSERT_NO_THROW(range.Prev());
		ASSERT_EQ(range.Get(), 'i');

		ASSERT_NO_THROW(range.Prev());
		ASSERT_EQ(range.Get(), 'r');

		ASSERT_NO_THROW(range.Prev());
		ASSERT_EQ(range.Get(), 't');

		ASSERT_NO_THROW(range.Prev());
		ASSERT_EQ(range.Get(), 'S');

		ASSERT_NO_THROW(range.Prev());
		ASSERT_EQ(range.Get(), 't');

		ASSERT_NO_THROW(range.Prev());
		ASSERT_EQ(range.Get(), 's');

		ASSERT_NO_THROW(range.Prev());
		ASSERT_EQ(range.Get(), 'e');

		ASSERT_NO_THROW(range.Prev());
		ASSERT_EQ(range.Get(), 'T');

		ASSERT_ANY_THROW(range.Prev());
	}

	{
		const std::string str("TestСтрока");
		Utf8IteratorRange<std::string> range(str, str.end());

		ASSERT_ANY_THROW(range.Get());
		ASSERT_ANY_THROW(range.Next());

		ASSERT_NO_THROW(range.Prev());
		ASSERT_EQ(range.Get(), L'а');

		ASSERT_NO_THROW(range.Prev());
		ASSERT_EQ(range.Get(), L'к');

		ASSERT_NO_THROW(range.Prev());
		ASSERT_EQ(range.Get(), L'о');

		ASSERT_NO_THROW(range.Prev());
		ASSERT_EQ(range.Get(), L'р');

		ASSERT_NO_THROW(range.Prev());
		ASSERT_EQ(range.Get(), L'т');

		ASSERT_NO_THROW(range.Prev());
		ASSERT_EQ(range.Get(), L'С');

		ASSERT_NO_THROW(range.Prev());
		ASSERT_EQ(range.Get(), 't');

		ASSERT_NO_THROW(range.Prev());
		ASSERT_EQ(range.Get(), 's');

		ASSERT_NO_THROW(range.Prev());
		ASSERT_EQ(range.Get(), 'e');

		ASSERT_NO_THROW(range.Prev());
		ASSERT_EQ(range.Get(), 'T');

		ASSERT_ANY_THROW(range.Prev());
	}

	{
		const std::string str("ТестString");
		Utf8IteratorRange<std::string> range(str, str.end());

		ASSERT_ANY_THROW(range.Get());
		ASSERT_ANY_THROW(range.Next());

		ASSERT_NO_THROW(range.Prev());
		ASSERT_EQ(range.Get(), 'g');

		ASSERT_NO_THROW(range.Prev());
		ASSERT_EQ(range.Get(), 'n');

		ASSERT_NO_THROW(range.Prev());
		ASSERT_EQ(range.Get(), 'i');

		ASSERT_NO_THROW(range.Prev());
		ASSERT_EQ(range.Get(), 'r');

		ASSERT_NO_THROW(range.Prev());
		ASSERT_EQ(range.Get(), 't');

		ASSERT_NO_THROW(range.Prev());
		ASSERT_EQ(range.Get(), 'S');

		ASSERT_NO_THROW(range.Prev());
		ASSERT_EQ(range.Get(), L'т');

		ASSERT_NO_THROW(range.Prev());
		ASSERT_EQ(range.Get(), L'с');

		ASSERT_NO_THROW(range.Prev());
		ASSERT_EQ(range.Get(), L'е');

		ASSERT_NO_THROW(range.Prev());
		ASSERT_EQ(range.Get(), L'Т');

		ASSERT_ANY_THROW(range.Prev());
	}

	{
		const std::string str("\x92\xa5\xe1\xe2\x91\xe2\xe0\xae\xaa\xa0"); // ТестСтрока in CP866
		Utf8IteratorRange<std::string> range(str, str.end());

		ASSERT_ANY_THROW(range.Prev());
	}
}
