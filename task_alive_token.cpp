#include <stingray/toolkit/task_alive_token.h>

namespace stingray
{

	void InvokeTask(const function<void ()>& task, const task_alive_token::ValueWeakPtr& tokenValue)
	{
		shared_ptr<Mutex> mutex;
		{
			task_alive_token::ValuePtr value = tokenValue.lock();
			if (!value)
				return;
			mutex = value->Sync;
		}

		MutexLock tl(*mutex);
		if (tokenValue.lock())
			task();
	}

}
