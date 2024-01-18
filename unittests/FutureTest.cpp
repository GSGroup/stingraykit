// Copyright (c) 2011 - 2024, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/function/bind.h>
#include <stingraykit/future.h>
#include <stingraykit/thread/Thread.h>

#include <gtest/gtest.h>

using namespace stingray;

namespace
{

	static void SetValueFunc(shared_ptr<promise<int> > myPromise)
	{
		Thread::Sleep(100);
		myPromise->set_value(42);
		myPromise.reset();
	}


	static void DontSetValueFunc(shared_ptr<promise<int> >& myPromise)
	{
		Thread::Sleep(100);
		myPromise.reset();
	}


	static void GetValueFunc(shared_future<int&> myFuture)
	{
		Thread::Sleep(rand() % 100);
		ASSERT_EQ(myFuture.get(), 35);
	}

}


TEST(FutureTest, HoldedOnPromises)
{
	shared_ptr<promise<int> > nero_promises(new promise<int>);
	ThreadPtr setter(new Thread("funcSetter", Bind(&SetValueFunc, nero_promises)));
	future<int> f(nero_promises->get_future());
	nero_promises.reset();
	f.wait();
	ASSERT_EQ(f.is_ready(), true);
	ASSERT_EQ(f.has_value(), true);
	ASSERT_EQ(f.has_exception(), false);
	ASSERT_EQ(f.get(), 42);
	setter.reset();
}


TEST(FutureTest, BrokenPromises)
{
	shared_ptr<promise<int> > broken_promise(new promise<int>);
	future<int> f = broken_promise->get_future();
	ThreadPtr nosetter(new Thread("funcDontSetter", Bind(&DontSetValueFunc, wrap_ref(broken_promise))));
	f.wait();
	ASSERT_EQ(f.has_value(), false);
	ASSERT_EQ(f.has_exception(), true);
	ASSERT_ANY_THROW(f.get());
}


TEST(FutureTest, MultipleReads)
{
	promise<int&> p;
	std::vector<ThreadPtr> readers;
	shared_future<int&> f = p.get_future().share();
	for (int i = 0; i < 10; ++i)
		readers.push_back(make_shared_ptr<Thread>("funcGetter", Bind(&GetValueFunc, shared_future<int&>(f))));
	int value = 35;
	p.set_value(value);
	readers.clear();
}
