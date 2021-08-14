#include <stingraykit/collection/Queue.h>

#include <stingraykit/function/bind.h>

#include <string>

#include <gtest/gtest.h>

using namespace stingray;

namespace
{

	typedef Queue<std::string>				QueueT;
	typedef std::deque<std::string>			DequeT;

	bool StringLengthGreater(const std::string& testee, size_t len)
	{ return testee.size() > len; }

}


TEST(QueueTest, Construction)
{
	{
		QueueT testee;
		EXPECT_TRUE(testee.empty());
	}
	{
		DequeT deq;
		deq.push_back("one");
		deq.push_back("two");
		deq.push_back("three");
		deq.push_back("four");

		QueueT testee(deq);
		EXPECT_EQ(testee.size(), (size_t)4);
	}
	{
		QueueT q;
		q.push(std::string("one"));
		q.push(std::string("two"));
		q.push(std::string("three"));
		q.push(std::string("four"));

		QueueT testee(q);
		EXPECT_EQ(testee.size(), (size_t)4);
	}
	{
		DequeT deq;
		deq.push_back("one");
		deq.push_back("two");
		deq.push_back("three");
		deq.push_back("four");

		QueueT testee(ToRange(deq));
		EXPECT_EQ(testee.size(), (size_t)4);
	}
}


TEST(QueueTest, PushPop)
{
	QueueT testee;
	testee.push(std::string("one"));
	testee.push(std::string("two"));
	testee.push(std::string("three"));
	testee.push(std::string("four"));

	optional<std::string> lastValue;
	lastValue = testee.pop();
	EXPECT_EQ(*lastValue, "one");
	lastValue = testee.pop();
	EXPECT_EQ(*lastValue, "two");
	lastValue = testee.pop();
	EXPECT_EQ(*lastValue, "three");
	lastValue = testee.pop();
	EXPECT_EQ(*lastValue, "four");
	lastValue = testee.pop();
	EXPECT_EQ(lastValue, null);
}


TEST(QueueTest, Erase)
{
	QueueT testee;
	testee.push(std::string("one"));
	testee.push(std::string("two"));
	testee.push(std::string("two"));
	testee.push(std::string("two"));
	testee.push(std::string("three"));
	testee.push(std::string("four"));

	EXPECT_TRUE(std::find(testee.begin(), testee.end(), std::string("two")) != testee.end());
	testee.erase(std::remove(testee.begin(), testee.end(), std::string("two")), testee.end());
	EXPECT_TRUE(std::find(testee.begin(), testee.end(), std::string("two")) == testee.end());
	EXPECT_EQ(testee.size(), (size_t)3);

	EXPECT_TRUE(std::find(testee.begin(), testee.end(), std::string("three")) != testee.end());
	EXPECT_TRUE(std::find(testee.begin(), testee.end(), std::string("four")) != testee.end());
	testee.erase(std::remove_if(testee.begin(), testee.end(), Bind(&StringLengthGreater, _1, 3)), testee.end());
	EXPECT_TRUE(std::find(testee.begin(), testee.end(), std::string("three")) == testee.end());
	EXPECT_TRUE(std::find(testee.begin(), testee.end(), std::string("four")) == testee.end());
	EXPECT_EQ(testee.size(), (size_t)1);
}


TEST(QueueTest, Clear)
{
	QueueT testee;
	testee.push(std::string("one"));
	testee.push(std::string("two"));
	testee.push(std::string("two"));
	testee.push(std::string("two"));
	testee.push(std::string("three"));
	testee.push(std::string("four"));

	EXPECT_EQ(testee.size(), (size_t)6);
	testee.clear();
	EXPECT_EQ(testee.size(), (size_t)0);
	EXPECT_TRUE(testee.empty());
}


TEST(QueueTest, Access)
{
	QueueT testee;
	testee.push(std::string("one"));
	testee.push(std::string("two"));
	testee.push(std::string("two"));
	testee.push(std::string("two"));
	testee.push(std::string("three"));
	testee.push(std::string("four"));

	EXPECT_EQ(std::string("one"), testee.front());
	EXPECT_EQ(std::string("four"), testee.back());
	testee.pop();
	EXPECT_EQ(std::string("two"), testee.front());
	EXPECT_EQ(std::string("four"), testee.back());

	testee.clear();
	EXPECT_ANY_THROW(testee.front());
	EXPECT_ANY_THROW(testee.back());
}
