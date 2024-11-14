// Copyright (c) 2011 - 2024, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/collection/EnumerableBuilder.h>
#include <stingraykit/collection/EnumerableHelpers.h>
#include <stingraykit/collection/ForEach.h>
#include <stingraykit/collection/StlEnumeratorAdapter.h>
#include <stingraykit/function/functional.h>

#include <unittests/EnumerableMatcher.h>

#include <gmock/gmock-more-matchers.h>

using namespace stingray;

using ::testing::ElementsAre;
using ::testing::IsEmpty;

namespace
{

	struct InvalidEnumerator : public virtual IEnumerator<bool>
	{
		bool Valid() const override { return true; }
		ItemType Get() const override { return true; }
		void Next() override { STINGRAYKIT_THROW("Must not iterate"); }
	};

	template < typename Range_ >
	void StartIterate(Range_ range)
	{
		for (bool i : range)
			break;
	}

	class ValueHolder
	{
	private:
		int		_value;

	public:
		ValueHolder(int value) : _value(value) { }

		int GetValue() const { return _value; }
		const int& GetValueRef() const { return _value; }

		bool operator == (const ValueHolder& other) const { return _value == other._value; }
	};
	STINGRAYKIT_DECLARE_PTR(ValueHolder);

	struct Base
	{
		virtual ~Base()
		{ }

		virtual int GetValue() const = 0;
	};
	STINGRAYKIT_DECLARE_PTR(Base);

	struct Derived1 : public Base
	{
		int _value;

		Derived1(int value) : _value(value)
		{ }

		int GetValue() const override
		{ return _value; }
	};
	STINGRAYKIT_DECLARE_PTR(Derived1);

	struct Derived2 : public Base
	{
		int _value;

		Derived2(int value) : _value(value)
		{ }

		int GetValue() const override
		{ return _value; }
	};
	STINGRAYKIT_DECLARE_PTR(Derived2);

}


TEST(EnumerableTest, ForBasedLoop)
{
	{
		const shared_ptr<IEnumerable<bool>> en = MakeSimpleEnumerable(MakeShared<InvalidEnumerator>());

		ASSERT_NO_THROW(StartIterate(IterableEnumerable(en)));
	}

	{
		const auto en = (EnumerableBuilder<int>() % 0 % 1 % 2 % 3 % 4).Get();

		int j = 0;
		for (int i : IterableEnumerable(en))
			ASSERT_EQ(j++, i);

		ASSERT_EQ(j, 5);
	}
}


TEST(EnumerableTest, ForEachLoop)
{
	const auto en = (EnumerableBuilder<BasePtr>()
			% make_shared_ptr<Derived1>(1)
			% make_shared_ptr<Derived2>(2)
			% make_shared_ptr<Derived1>(3)
			% make_shared_ptr<Derived2>(4)
			% make_shared_ptr<Derived1>(5)
			% make_shared_ptr<Derived2>(6)
			% make_shared_ptr<Derived1>(7)
			% make_shared_ptr<Derived2>(8)
			% make_shared_ptr<Derived1>(9)
			% make_shared_ptr<Derived2>(10)).Get();

	int value;

	value = 1;
	FOR_EACH(const BasePtr base IN en)
	{
		ASSERT_EQ(base->GetValue(), value);
		value++;
	}

	value = 2;
	FOR_EACH(const BasePtr base IN en WHERE base->GetValue() % 2 == 0)
	{
		ASSERT_EQ(base->GetValue(), value);
		value += 2;
	}

	value = 1;
	FOR_EACH(const Derived1Ptr derived1 IN en)
	{
		ASSERT_EQ(derived1->GetValue(), value);
		value += 2;
	}

	value = 1;
	FOR_EACH(const Derived1Ptr derived1 IN en WHERE (derived1->GetValue() + 1) % 2 == 0)
	{
		ASSERT_EQ(derived1->GetValue(), value);
		value += 2;
	}

	value = 2;
	FOR_EACH(const Derived2Ptr derived2 IN en)
	{
		ASSERT_EQ(derived2->GetValue(), value);
		value += 2;
	}

	value = 2;
	FOR_EACH(const Derived2Ptr derived2 IN en WHERE derived2->GetValue() % 2 == 0)
	{
		ASSERT_EQ(derived2->GetValue(), value);
		value += 2;
	}
}


TEST(EnumerableTest, Aggregate)
{
	{
		const auto en = EnumerableBuilder<int>().Get();

		ASSERT_EQ(Enumerable::Aggregate(en, std::plus<int>()), null);
		ASSERT_EQ(Enumerable::Aggregate(en, 10, std::plus<int>()), 10);
	}

	{
		const auto en = (EnumerableBuilder<int>() % 1).Get();

		ASSERT_EQ(Enumerable::Aggregate(en, std::plus<int>()), 1);
		ASSERT_EQ(Enumerable::Aggregate(en, 10, std::plus<int>()), 11);
	}

	{
		const auto en = (EnumerableBuilder<int>() % 1 % 2).Get();

		ASSERT_EQ(Enumerable::Aggregate(en, std::plus<int>()), 3);
		ASSERT_EQ(Enumerable::Aggregate(en, 10, std::plus<int>()), 13);
	}

	{
		const auto en = (EnumerableBuilder<int>() % 1 % 2 % 3 % 4 % 5).Get();

		ASSERT_EQ(Enumerable::Aggregate(en, std::plus<int>()), 15);
		ASSERT_EQ(Enumerable::Aggregate(en, 10, std::plus<int>()), 25);
	}
}


TEST(EnumerableTest, Concat)
{
	{
		const auto en1 = EnumerableBuilder<int>().Get();
		const auto en2 = EnumerableBuilder<int>().Get();

		ASSERT_THAT(Enumerable::Concat(en1, en2), MatchEnumerable(IsEmpty()));
		ASSERT_THAT(Enumerable::Concat(en1->GetEnumerator(), en2->GetEnumerator()), MatchEnumerable(IsEmpty()));

		ASSERT_THAT(Enumerable::Concat(en2, en1), MatchEnumerable(IsEmpty()));
		ASSERT_THAT(Enumerable::Concat(en2->GetEnumerator(), en1->GetEnumerator()), MatchEnumerable(IsEmpty()));
	}

	{
		const auto en1 = (EnumerableBuilder<int>() % 1).Get();
		const auto en2 = EnumerableBuilder<int>().Get();

		ASSERT_THAT(Enumerable::Concat(en1, en2), MatchEnumerable(ElementsAre(1)));
		ASSERT_THAT(Enumerable::Concat(en1->GetEnumerator(), en2->GetEnumerator()), MatchEnumerable(ElementsAre(1)));

		ASSERT_THAT(Enumerable::Concat(en2, en1), MatchEnumerable(ElementsAre(1)));
		ASSERT_THAT(Enumerable::Concat(en2->GetEnumerator(), en1->GetEnumerator()), MatchEnumerable(ElementsAre(1)));
	}

	{
		const auto en1 = (EnumerableBuilder<int>() % 1).Get();
		const auto en2 = (EnumerableBuilder<int>() % 2).Get();

		ASSERT_THAT(Enumerable::Concat(en1, en2), MatchEnumerable(ElementsAre(1, 2)));
		ASSERT_THAT(Enumerable::Concat(en1->GetEnumerator(), en2->GetEnumerator()), MatchEnumerable(ElementsAre(1, 2)));

		ASSERT_THAT(Enumerable::Concat(en2, en1), MatchEnumerable(ElementsAre(2, 1)));
		ASSERT_THAT(Enumerable::Concat(en2->GetEnumerator(), en1->GetEnumerator()), MatchEnumerable(ElementsAre(2, 1)));
	}

	{
		const auto en1 = EnumerableBuilder<int>().Get();
		const auto en2 = (EnumerableBuilder<int>() % 1 % 2).Get();

		ASSERT_THAT(Enumerable::Concat(en1, en2), MatchEnumerable(ElementsAre(1, 2)));
		ASSERT_THAT(Enumerable::Concat(en1->GetEnumerator(), en2->GetEnumerator()), MatchEnumerable(ElementsAre(1, 2)));

		ASSERT_THAT(Enumerable::Concat(en2, en1), MatchEnumerable(ElementsAre(1, 2)));
		ASSERT_THAT(Enumerable::Concat(en2->GetEnumerator(), en1->GetEnumerator()), MatchEnumerable(ElementsAre(1, 2)));
	}

	{
		const auto en1 = (EnumerableBuilder<int>() % 1 % 2).Get();
		const auto en2 = (EnumerableBuilder<int>() % 3 % 4 % 5).Get();

		ASSERT_THAT(Enumerable::Concat(en1, en2), MatchEnumerable(ElementsAre(1, 2, 3, 4, 5)));
		ASSERT_THAT(Enumerable::Concat(en1->GetEnumerator(), en2->GetEnumerator()), MatchEnumerable(ElementsAre(1, 2, 3, 4, 5)));

		ASSERT_THAT(Enumerable::Concat(en2, en1), MatchEnumerable(ElementsAre(3, 4, 5, 1, 2)));
		ASSERT_THAT(Enumerable::Concat(en2->GetEnumerator(), en1->GetEnumerator()), MatchEnumerable(ElementsAre(3, 4, 5, 1, 2)));
	}
}


TEST(EnumerableTest, Contains)
{
	{
		const auto en = (EnumerableBuilder<int>() % 0 % 1 % 2 % 3 % 4).Get();

		ASSERT_TRUE(Enumerable::Contains(en, 2));
		ASSERT_FALSE(Enumerable::Contains(en, 5));
	}

	{
		const auto en = (EnumerableBuilder<ValueHolderPtr>() % make_shared_ptr<ValueHolder>(0) % make_shared_ptr<ValueHolder>(1) % make_shared_ptr<ValueHolder>(2) % make_shared_ptr<ValueHolder>(3) % make_shared_ptr<ValueHolder>(4)).Get();

		ASSERT_TRUE(Enumerable::Contains(en | Transform(&ValueHolder::GetValue), 2));
		ASSERT_FALSE(Enumerable::Contains(en | Transform(&ValueHolder::GetValue), 5));

		ASSERT_TRUE(Enumerable::Contains(en | Transform(&ValueHolder::GetValueRef), 2));
		ASSERT_FALSE(Enumerable::Contains(en | Transform(&ValueHolder::GetValueRef), 5));
	}

	{
		const auto en = (EnumerableBuilder<ValueHolder>() % ValueHolder(0) % ValueHolder(1) % ValueHolder(2) % ValueHolder(3) % ValueHolder(4)).Get();

		ASSERT_TRUE(Enumerable::Contains(en, 2));
		ASSERT_FALSE(Enumerable::Contains(en, 5));
	}
}


TEST(EnumerableTest, ElementAt)
{
	{
		const auto en = EnumerableBuilder<int>().Get();

		ASSERT_THROW(Enumerable::ElementAt(en, 0), IndexOutOfRangeException);
		ASSERT_THROW(Enumerable::ElementAt(en, 1), IndexOutOfRangeException);
		ASSERT_THROW(Enumerable::ElementAt(en, 4), IndexOutOfRangeException);
		ASSERT_THROW(Enumerable::ElementAt(en, 5), IndexOutOfRangeException);

		ASSERT_EQ(Enumerable::ElementAtOrDefault(en, 0), null);
		ASSERT_EQ(Enumerable::ElementAtOrDefault(en, 1), null);
		ASSERT_EQ(Enumerable::ElementAtOrDefault(en, 4), null);
		ASSERT_EQ(Enumerable::ElementAtOrDefault(en, 5), null);

		ASSERT_EQ(Enumerable::ElementAtOrDefault(en, 0, 12345), 12345);
		ASSERT_EQ(Enumerable::ElementAtOrDefault(en, 1, 12345), 12345);
		ASSERT_EQ(Enumerable::ElementAtOrDefault(en, 4, 12345), 12345);
		ASSERT_EQ(Enumerable::ElementAtOrDefault(en, 5, 12345), 12345);
	}

	{
		const auto en = (EnumerableBuilder<int>() % 1).Get();

		ASSERT_EQ(Enumerable::ElementAt(en, 0), 1);
		ASSERT_THROW(Enumerable::ElementAt(en, 1), IndexOutOfRangeException);
		ASSERT_THROW(Enumerable::ElementAt(en, 4), IndexOutOfRangeException);
		ASSERT_THROW(Enumerable::ElementAt(en, 5), IndexOutOfRangeException);

		ASSERT_EQ(Enumerable::ElementAtOrDefault(en, 0), 1);
		ASSERT_EQ(Enumerable::ElementAtOrDefault(en, 1), null);
		ASSERT_EQ(Enumerable::ElementAtOrDefault(en, 4), null);
		ASSERT_EQ(Enumerable::ElementAtOrDefault(en, 5), null);

		ASSERT_EQ(Enumerable::ElementAtOrDefault(en, 0, 12345), 1);
		ASSERT_EQ(Enumerable::ElementAtOrDefault(en, 1, 12345), 12345);
		ASSERT_EQ(Enumerable::ElementAtOrDefault(en, 4, 12345), 12345);
		ASSERT_EQ(Enumerable::ElementAtOrDefault(en, 5, 12345), 12345);
	}

	{
		const auto en = (EnumerableBuilder<int>() % 1 % 2 % 3 % 4 % 5).Get();

		ASSERT_EQ(Enumerable::ElementAt(en, 0), 1);
		ASSERT_EQ(Enumerable::ElementAt(en, 1), 2);
		ASSERT_EQ(Enumerable::ElementAt(en, 2), 3);
		ASSERT_EQ(Enumerable::ElementAt(en, 3), 4);
		ASSERT_EQ(Enumerable::ElementAt(en, 4), 5);
		ASSERT_THROW(Enumerable::ElementAt(en, 5), IndexOutOfRangeException);

		ASSERT_EQ(Enumerable::ElementAtOrDefault(en, 0), 1);
		ASSERT_EQ(Enumerable::ElementAtOrDefault(en, 1), 2);
		ASSERT_EQ(Enumerable::ElementAtOrDefault(en, 2), 3);
		ASSERT_EQ(Enumerable::ElementAtOrDefault(en, 3), 4);
		ASSERT_EQ(Enumerable::ElementAtOrDefault(en, 4), 5);
		ASSERT_EQ(Enumerable::ElementAtOrDefault(en, 5), null);

		ASSERT_EQ(Enumerable::ElementAtOrDefault(en, 0, 12345), 1);
		ASSERT_EQ(Enumerable::ElementAtOrDefault(en, 1, 12345), 2);
		ASSERT_EQ(Enumerable::ElementAtOrDefault(en, 2, 12345), 3);
		ASSERT_EQ(Enumerable::ElementAtOrDefault(en, 3, 12345), 4);
		ASSERT_EQ(Enumerable::ElementAtOrDefault(en, 4, 12345), 5);
		ASSERT_EQ(Enumerable::ElementAtOrDefault(en, 5, 12345), 12345);
	}
}


TEST(EnumerableTest, DefaultIfEmpty)
{
	{
		const auto en = EnumerableBuilder<int>().Get();

		ASSERT_THAT(Enumerable::DefaultIfEmpty(en), MatchEnumerable(ElementsAre(int())));
		ASSERT_THAT(Enumerable::DefaultIfEmpty(en->GetEnumerator()), MatchEnumerable(ElementsAre(int())));

		ASSERT_THAT(Enumerable::DefaultIfEmpty(en, 12345), MatchEnumerable(ElementsAre(12345)));
		ASSERT_THAT(Enumerable::DefaultIfEmpty(en->GetEnumerator(), 12345), MatchEnumerable(ElementsAre(12345)));
	}

	{
		const auto en = (EnumerableBuilder<int>() % 1).Get();

		ASSERT_THAT(Enumerable::DefaultIfEmpty(en), MatchEnumerable(ElementsAre(1)));
		ASSERT_THAT(Enumerable::DefaultIfEmpty(en->GetEnumerator()), MatchEnumerable(ElementsAre(1)));

		ASSERT_THAT(Enumerable::DefaultIfEmpty(en, 12345), MatchEnumerable(ElementsAre(1)));
		ASSERT_THAT(Enumerable::DefaultIfEmpty(en->GetEnumerator(), 12345), MatchEnumerable(ElementsAre(1)));
	}

	{
		const auto en = (EnumerableBuilder<int>() % 1 % 2 % 3 % 4 % 5).Get();

		ASSERT_THAT(Enumerable::DefaultIfEmpty(en), MatchEnumerable(ElementsAre(1, 2, 3, 4, 5)));
		ASSERT_THAT(Enumerable::DefaultIfEmpty(en->GetEnumerator()), MatchEnumerable(ElementsAre(1, 2, 3, 4, 5)));

		ASSERT_THAT(Enumerable::DefaultIfEmpty(en, 12345), MatchEnumerable(ElementsAre(1, 2, 3, 4, 5)));
		ASSERT_THAT(Enumerable::DefaultIfEmpty(en->GetEnumerator(), 12345), MatchEnumerable(ElementsAre(1, 2, 3, 4, 5)));
	}
}


TEST(EnumerableTest, First)
{
	{
		const auto en = EnumerableBuilder<int>().Get();

		ASSERT_THROW(Enumerable::First(en), InvalidOperationException);
		ASSERT_THROW(en | First(), InvalidOperationException);

		ASSERT_THROW(Enumerable::First(en, Bind(comparers::Greater(), _1, 1)), InvalidOperationException);

		ASSERT_EQ(Enumerable::FirstOrDefault(en), null);
		ASSERT_EQ(en | FirstOrDefault(), null);

		ASSERT_EQ(Enumerable::FirstOrDefault(en, Bind(comparers::Greater(), _1, 1)), null);

		ASSERT_EQ(Enumerable::FirstOrDefault(en, 12345), 12345);
		ASSERT_EQ(Enumerable::FirstOrDefault(en, Bind(comparers::Greater(), _1, 1), 12345), 12345);
	}

	{
		const auto en = (EnumerableBuilder<int>() % 1).Get();

		ASSERT_EQ(Enumerable::First(en), 1);
		ASSERT_EQ(en | First(), 1);

		ASSERT_THROW(Enumerable::First(en, Bind(comparers::Greater(), _1, 1)), InvalidOperationException);

		ASSERT_EQ(Enumerable::FirstOrDefault(en), 1);
		ASSERT_EQ(en | FirstOrDefault(), 1);

		ASSERT_EQ(Enumerable::FirstOrDefault(en, Bind(comparers::Greater(), _1, 1)), null);

		ASSERT_EQ(Enumerable::FirstOrDefault(en, 12345), 1);
		ASSERT_EQ(Enumerable::FirstOrDefault(en, Bind(comparers::Greater(), _1, 1), 12345), 12345);
	}

	{
		const auto en = (EnumerableBuilder<int>() % 1 % 2 % 3 % 4 % 5).Get();

		ASSERT_EQ(Enumerable::First(en), 1);
		ASSERT_EQ(en | First(), 1);

		ASSERT_EQ(Enumerable::First(en, Bind(comparers::Greater(), _1, 1)), 2);

		ASSERT_EQ(Enumerable::FirstOrDefault(en), 1);
		ASSERT_EQ(en | FirstOrDefault(), 1);

		ASSERT_EQ(Enumerable::FirstOrDefault(en, Bind(comparers::Greater(), _1, 1)), 2);

		ASSERT_EQ(Enumerable::FirstOrDefault(en, 12345), 1);
		ASSERT_EQ(Enumerable::FirstOrDefault(en, Bind(comparers::Greater(), _1, 1), 12345), 2);
	}
}


TEST(EnumerableTest, Last)
{
	{
		const auto en = EnumerableBuilder<int>().Get();

		ASSERT_THROW(Enumerable::Last(en), InvalidOperationException);
		ASSERT_THROW(Enumerable::Last(en, Bind(comparers::Less(), _1, 5)), InvalidOperationException);

		ASSERT_EQ(Enumerable::LastOrDefault(en), null);
		ASSERT_EQ(Enumerable::LastOrDefault(en, Bind(comparers::Less(), _1, 5)), null);

		ASSERT_EQ(Enumerable::LastOrDefault(en, 12345), 12345);
		ASSERT_EQ(Enumerable::LastOrDefault(en, Bind(comparers::Less(), _1, 5), 12345), 12345);
	}

	{
		const auto en = (EnumerableBuilder<int>() % 1).Get();

		ASSERT_EQ(Enumerable::Last(en), 1);
		ASSERT_EQ(Enumerable::Last(en, Bind(comparers::Less(), _1, 5)), 1);

		ASSERT_EQ(Enumerable::LastOrDefault(en), 1);
		ASSERT_EQ(Enumerable::LastOrDefault(en, Bind(comparers::Less(), _1, 5)), 1);

		ASSERT_EQ(Enumerable::LastOrDefault(en, 12345), 1);
		ASSERT_EQ(Enumerable::LastOrDefault(en, Bind(comparers::Less(), _1, 5), 12345), 1);
	}

	{
		const auto en = (EnumerableBuilder<int>() % 1 % 2 % 3 % 4 % 5).Get();

		ASSERT_EQ(Enumerable::Last(en), 5);
		ASSERT_EQ(Enumerable::Last(en, Bind(comparers::Less(), _1, 5)), 4);

		ASSERT_EQ(Enumerable::LastOrDefault(en), 5);
		ASSERT_EQ(Enumerable::LastOrDefault(en, Bind(comparers::Less(), _1, 5)), 4);

		ASSERT_EQ(Enumerable::LastOrDefault(en, 12345), 5);
		ASSERT_EQ(Enumerable::LastOrDefault(en, Bind(comparers::Less(), _1, 5), 12345), 4);
	}
}


TEST(EnumerableTest, Polymorphic)
{
	{
		const auto en = (EnumerableBuilder<BasePtr>()
				% make_shared_ptr<Derived1>(1)
				% make_shared_ptr<Derived1>(2)
				% make_shared_ptr<Derived1>(3)
				% make_shared_ptr<Derived1>(42)).Get();

		ASSERT_THAT(
				en | Cast<Derived1Ptr>() | Transform(&Derived1::GetValue),
				MatchEnumerable(ElementsAre(1, 2, 3, 42)));

		ASSERT_TRUE(Enumerable::Any(en | Cast<Derived2Ptr>()));
		ASSERT_ANY_THROW(Enumerable::First(en | Cast<Derived2Ptr>()));
	}

	{
		const auto en = (EnumerableBuilder<BasePtr>()
				% make_shared_ptr<Derived1>(1)
				% make_shared_ptr<Derived1>(2)
				% make_shared_ptr<Derived1>(3)
				% make_shared_ptr<Derived1>(42)
				% make_shared_ptr<Derived2>(53)
				% make_shared_ptr<Derived2>(1)
				% make_shared_ptr<Derived1>(37)
				% make_shared_ptr<Derived2>(7)).Get();

		ASSERT_THAT(
			en | OfType<Derived1Ptr>() | Transform(&Derived1::GetValue),
			MatchEnumerable(ElementsAre(1, 2, 3, 42, 37)));

		ASSERT_THAT(
			en | OfType<Derived2Ptr>() | Transform(&Derived2::GetValue),
			MatchEnumerable(ElementsAre(53, 1, 7)));
	}
}


TEST(EnumerableTest, Clone)
{
	{
		const auto en = EnumerableBuilder<int>().Get();
		const auto cloned = en | Clone();

		ASSERT_NE(en.get(), cloned.get());
		ASSERT_EQ(en.use_count(), 1);
		ASSERT_EQ(cloned.use_count(), 1);
		ASSERT_THAT(cloned, MatchEnumerable(IsEmpty()));
	}

	{
		const auto en = (EnumerableBuilder<int>() % 0 % 1 % 2 % 3 % 4).Get();
		const auto cloned = en | Clone();

		ASSERT_NE(en.get(), cloned.get());
		ASSERT_EQ(en.use_count(), 1);
		ASSERT_EQ(cloned.use_count(), 1);
		ASSERT_THAT(cloned, MatchEnumerable(ElementsAre(0, 1, 2, 3, 4)));
	}

	{
		const auto en = (EnumerableBuilder<std::string>() % "0" % "" % "2" % "" % "4").Get();
		const auto cloned = en | Filter(not_(&std::string::empty)) | Clone();

		ASSERT_NE(en.get(), cloned.get());
		ASSERT_EQ(en.use_count(), 1);
		ASSERT_EQ(cloned.use_count(), 1);
		ASSERT_THAT(cloned, MatchEnumerable(ElementsAre("0", "2", "4")));
	}
}
