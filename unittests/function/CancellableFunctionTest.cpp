// Copyright (c) 2011 - 2024, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

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
