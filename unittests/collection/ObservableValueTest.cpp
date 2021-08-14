#include <stingraykit/collection/EnumerableHelpers.h>
#include <stingraykit/compare/CollectionComparer.h>
#include <stingraykit/ObservableValue.h>
#include <stingraykit/signal/ValueFromSignalObtainer.h>

#include <gtest/gtest.h>

using namespace stingray;

typedef ObservableValue<int, comparers::Equals, ObservableValuePolicies::DisabledPopulation> DisabledObservableInt;

typedef ObservableValue<int> ObservableInt;
typedef ObservableValue<int, comparers::Equals, ObservableValuePolicies::ConditionalPopulation> ConditionalObservableInt;

typedef ObservableValue<shared_ptr<IEnumerable<int> >, comparers::CmpToEquals<CollectionCmp> > ObservableIntEnumerable;
typedef ObservableValue<shared_ptr<IEnumerable<int> >, comparers::CmpToEquals<CollectionCmp>, ObservableValuePolicies::ConditionalPopulation> ConditionalObservableIntEnumerable;

TEST(ObservableValueTest, Disabled)
{
	DisabledObservableInt x;
	ASSERT_FALSE(HasValueInSignal(x.OnChanged()));
	x = 1;
	ASSERT_FALSE(HasValueInSignal(x.OnChanged()));
}

TEST(ObservableValueTest, Mandatory)
{
	ObservableInt x;
	ASSERT_EQ(GetValueFromSignal(x.OnChanged()), 0);
	x = 1;
	ASSERT_NE(GetValueFromSignal(x.OnChanged()), 0);
}

TEST(ObservableValueTest, Conditional)
{
	ConditionalObservableInt x;
	ASSERT_FALSE(HasValueInSignal(x.OnChanged()));
	x = 1;
	ASSERT_TRUE(HasValueInSignal(x.OnChanged()));
}

TEST(ObservableValueTest, ConditionalWithInitializer)
{
	ConditionalObservableInt x(0, ObservableValuePolicies::PopulateIfDiffers(1));
	ASSERT_TRUE(HasValueInSignal(x.OnChanged()));
	x = 1;
	ASSERT_FALSE(HasValueInSignal(x.OnChanged()));
}

TEST(ObservableValueTest, MandatoryEnumerable)
{
	ObservableIntEnumerable x;
	ASSERT_EQ(GetValueFromSignal(x.OnChanged()), shared_ptr<IEnumerable<int> >());
	x = MakeEmptyEnumerable();
	ASSERT_NE(GetValueFromSignal(x.OnChanged()), shared_ptr<IEnumerable<int> >());
}

TEST(ObservableValueTest, ConditionalEnumerable)
{
	ConditionalObservableIntEnumerable x;
	ASSERT_FALSE(HasValueInSignal(x.OnChanged()));
	x = MakeEmptyEnumerable();
	ASSERT_TRUE(HasValueInSignal(x.OnChanged()));
}

TEST(ObservableValueTest, ConditionalEnumerableWithInitializer)
{
	ConditionalObservableIntEnumerable x(MakeEmptyEnumerable(), ObservableValuePolicies::PopulateIfDiffers(MakeEmptyEnumerable()));
	ASSERT_FALSE(HasValueInSignal(x.OnChanged()));
	x = MakeOneItemEnumerable(0);
	ASSERT_TRUE(HasValueInSignal(x.OnChanged()));
}
