#include <stingraykit/metaprogramming/TypeList.h>

#include <gtest/gtest.h>

using namespace stingray;

TEST(TypeListTest, MaxElement)
{
	typedef TypeList<integral_constant<size_t, 3>, integral_constant<size_t, 2>, integral_constant<size_t, 7>, integral_constant<size_t, 10>, integral_constant<size_t, 1> > Numbers;
	static_assert(MaxElement<Numbers, integral_constant_less>::ValueT::Value == 10, "Max element not working");
}
