// Copyright (c) 2011 - 2024, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <unittests/Dummy.h>

#include <stingraykit/shared_ptr.h>
#include <stingraykit/self_counter.h>

#include <gtest/gtest.h>

#if defined(__clang__)
#pragma clang diagnostic ignored "-Wself-assign-overloaded"
#endif

using namespace stingray;

class PointersTest : public testing::Test, public FireRange {
protected:
	STINGRAYKIT_DECLARE_PTR(Dummy);

	struct EvilDummy : public Dummy, public self_counter<EvilDummy>
	{
		EvilDummy(FireRange *parent, bool verbose = false): Dummy(parent, verbose) {}
	};
	STINGRAYKIT_DECLARE_SELF_COUNT_PTR(EvilDummy);

	struct DeleterTester
	{
	private:
		int& _dtorInvokations;
		int& _deleterInvokations;

	public:
		DeleterTester(int& dtorInvokations, int& deleterInvokations) :
			_dtorInvokations(dtorInvokations), _deleterInvokations(deleterInvokations)
		{}

		~DeleterTester()
		{ ++_dtorInvokations; }

		void DeleterInvoked()
		{ ++_deleterInvokations; }
	};
	STINGRAYKIT_DECLARE_PTR(DeleterTester);

protected:
	static void DoDelete(DeleterTester* ptr)
	{
		ptr->DeleterInvoked();
		delete ptr;
	}
};


TEST_F(PointersTest, SafeBool)
{
	Counter = 0;
	DummyPtr ptr;
	ASSERT_EQ((bool)ptr, false);
	ptr.reset(new Dummy(this));
	ASSERT_EQ((bool)ptr, true);
	ptr.reset();
	ASSERT_EQ((bool)ptr, false);
}


TEST_F(PointersTest, ReferenceCount)
{
	Counter = 0;
	DummyPtr ptr, ptr2;
	ASSERT_EQ(Counter, 0);
	ASSERT_EQ(ptr.use_count(), 0u);
	ptr2 = ptr;
	ASSERT_EQ(ptr.use_count(), 0u);
	ptr.reset(new Dummy(this));
	ASSERT_EQ(ptr.unique(), true);
	ASSERT_EQ(ptr.use_count(), 1u);
	ptr2 = ptr;
	ASSERT_EQ(ptr.use_count(), 2u);
	ASSERT_EQ(Counter, 1);
}


TEST_F(PointersTest, Assignment)
{
	Counter = 0;
	{
		DummyPtr ptr(new Dummy(this)), ptr2(new Dummy(this)), ptr3;
		ASSERT_EQ(Counter, 2);
		ptr = ptr;
		ptr2 = ptr2;
		ptr3 = ptr;
		ASSERT_EQ(Counter, 2);
		ptr.reset();
		ASSERT_EQ(Counter, 2);
		std::swap(ptr, ptr3);
		ASSERT_EQ(Counter, 2);
		ASSERT_EQ((bool)ptr, true);
		ASSERT_EQ((bool)ptr3, false);

		ptr		= ptr;
		ptr2	= ptr2;
		ptr3	= ptr3;
	}
	ASSERT_EQ(Counter, 0);
}


TEST_F(PointersTest, Deleter)
{
	int dtor_invokations = 0;
	int deleter_invokations = 0;
	DeleterTesterPtr tester(new DeleterTester(dtor_invokations, deleter_invokations), &DoDelete);
	tester.reset();
	ASSERT_EQ(dtor_invokations, 1);
	ASSERT_EQ(deleter_invokations, 1);
}


TEST_F(PointersTest, WeakSelfLock)
{
	Counter = 0;
	DummyPtr ptr(new Dummy(this));
	ASSERT_EQ(Counter, 1);
	DummyWeakPtr weak(ptr);
	ptr = weak.lock();
	ASSERT_EQ((bool)ptr, true);
}


TEST_F(PointersTest, Weak)
{
	Counter = 0;
	DummyPtr ptr(new Dummy(this));
	ASSERT_EQ(Counter, 1);
	DummyWeakPtr weak;
	ASSERT_EQ(Counter, 1);
	weak = ptr;
	weak = weak;
	ASSERT_EQ(Counter, 1);
	{
		DummyPtr ptr2 = weak.lock();
		ASSERT_EQ(Counter, 1);
		ASSERT_EQ((bool)ptr2, true);
	}
	ptr.reset();
	ptr = weak.lock();
	ASSERT_EQ((bool)ptr, false);
	ASSERT_EQ(Counter, 0);
}


TEST_F(PointersTest, SelfCount)
{
	Counter = 0;
	{
		EvilDummySelfCountPtr x;
		ASSERT_EQ(Counter, 0);
		EvilDummySelfCountPtr y(new EvilDummy(this));
		ASSERT_EQ(Counter, 1);
		x = y;
		ASSERT_EQ(Counter, 1);
		y.reset();
		ASSERT_EQ(Counter, 1);
		x = x;
		y = y;
	}
	ASSERT_EQ(Counter, 0);
}
