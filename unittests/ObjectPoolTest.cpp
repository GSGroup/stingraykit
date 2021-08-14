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
