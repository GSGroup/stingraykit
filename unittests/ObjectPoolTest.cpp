// Copyright (c) 2011 - 2024, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/thread/Thread.h>
#include <stingraykit/time/ElapsedTime.h>
#include <stingraykit/ObjectPool.h>

#include <gtest/gtest.h>

using namespace stingray;

namespace
{

	class SomeObject
	{
	private:
		bool&			_threadNameError;
		std::string		_threadName;

	public:
		SomeObject(bool& threadNameError, const std::string& threadName) :
			_threadNameError(threadNameError),
			_threadName(threadName)
		{ }

		~SomeObject()
		{
			if (Thread::GetCurrentThreadName() != _threadName)
				_threadNameError = true;
		}
	};
	STINGRAYKIT_DECLARE_PTR(SomeObject);


	void GcFunc(ObjectPool& objectPool)
	{
		ElapsedTime t;
		while (t.ElapsedMilliseconds() < 3000)
			objectPool.CollectGarbage();
	}

}


TEST(ObjectPoolTest, ObjectDeletion)
{
	ObjectPool object_pool;
	bool thread_name_error = false;

	ThreadPtr th = make_shared_ptr<Thread>("gcThread", Bind(&GcFunc, wrap_ref(object_pool)));

	ElapsedTime t;
	SomeObjectWeakPtr weak_o;
	while (t.ElapsedMilliseconds() < 2500)
	{
		SomeObjectPtr o(new SomeObject(thread_name_error, "gcThread"));
		weak_o = o;
		object_pool.AddObject(o);
		o.reset();
		o = weak_o.lock();
	}

	th.reset();

	ASSERT_TRUE(!thread_name_error);
}
