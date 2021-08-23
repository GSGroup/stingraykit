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
