// Copyright (c) 2011 - 2024, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

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
