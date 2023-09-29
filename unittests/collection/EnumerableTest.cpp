#include <stingraykit/collection/EnumerableBuilder.h>
#include <stingraykit/collection/EnumerableHelpers.h>
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

}


TEST(EnumerableTest, ForBasedLoop)
{
	{
		const shared_ptr<IEnumerable<bool>> en = MakeSimpleEnumerable(MakeShared<InvalidEnumerator>());

		ASSERT_NO_THROW(StartIterate(IterableEnumerable(en)));
	}

	{
		EnumerableBuilder<int> b;
		b % 0 % 1 % 2 % 3 % 4;
		int j = 0;
		for (int i : IterableEnumerable(b.Get()))
			ASSERT_EQ(j++, i);

		ASSERT_EQ(j, 5);
	}
}


TEST(EnumerableTest, Contains)
{
	{
		EnumerableBuilder<int> b;
		b % 0 % 1 % 2 % 3 % 4;

		ASSERT_TRUE(Enumerable::Contains(b.Get(), 2));
		ASSERT_FALSE(Enumerable::Contains(b.Get(), 5));
	}

	{
		EnumerableBuilder<ValueHolderPtr> b;
		b % make_shared_ptr<ValueHolder>(0) % make_shared_ptr<ValueHolder>(1) % make_shared_ptr<ValueHolder>(2) % make_shared_ptr<ValueHolder>(3) % make_shared_ptr<ValueHolder>(4);

		ASSERT_TRUE(Enumerable::Contains(b.Get() | Transform(&ValueHolder::GetValue), 2));
		ASSERT_FALSE(Enumerable::Contains(b.Get() | Transform(&ValueHolder::GetValue), 5));

		ASSERT_TRUE(Enumerable::Contains(b.Get() | Transform(&ValueHolder::GetValueRef), 2));
		ASSERT_FALSE(Enumerable::Contains(b.Get() | Transform(&ValueHolder::GetValueRef), 5));
	}

	{
		EnumerableBuilder<ValueHolder> b;
		b % ValueHolder(0) % ValueHolder(1) % ValueHolder(2) % ValueHolder(3) % ValueHolder(4);

		ASSERT_TRUE(Enumerable::Contains(b.Get(), 2));
		ASSERT_FALSE(Enumerable::Contains(b.Get(), 5));
	}
}
