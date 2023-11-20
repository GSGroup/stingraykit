#include <stingraykit/collection/EnumerableBuilder.h>
#include <stingraykit/collection/EnumerableHelpers.h>
#include <stingraykit/collection/ForEach.h>
#include <stingraykit/collection/StlEnumeratorAdapter.h>

#include <gtest/gtest.h>

using namespace stingray;

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
