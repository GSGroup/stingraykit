#include <stingraykit/function/CancellableFunction.h>
#include <stingraykit/function/bind.h>

#include <gtest/gtest.h>

using namespace stingray;

namespace
{

	void FunctionWithoutRetval(bool& flag)
	{ flag = true; }


	bool FunctionWithRetval()
	{ return true; }

}


TEST(CancellableFunctionTest, WithoutRetval)
{
	TaskLifeToken taskLifeToken;
	bool flag = false;

	const function<void (bool& flag)> func = MakeCancellableFunction(&FunctionWithoutRetval, taskLifeToken.GetExecutionTester());

	func(flag);
	ASSERT_EQ(flag, true);

	taskLifeToken.Release();
	flag = false;

	func(flag);
	ASSERT_EQ(flag, false);
}

TEST(CancellableFunctionTest, WithRetval)
{
	TaskLifeToken taskLifeToken;
	bool flag = false;

	const function<bool ()> func = MakeCancellableFunction(&FunctionWithRetval, taskLifeToken.GetExecutionTester());

	ASSERT_NO_THROW(flag = func());
	ASSERT_EQ(flag, true);

	taskLifeToken.Release();

	ASSERT_ANY_THROW(func());
}
