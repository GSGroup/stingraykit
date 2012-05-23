#ifndef __GS_STINGRAY_TOOLKIT_TASK_ALIVE_TOKEN_H__
#define __GS_STINGRAY_TOOLKIT_TASK_ALIVE_TOKEN_H__


#include <stingray/toolkit/shared_ptr.h>
#include <stingray/threads/Thread.h>


namespace stingray
{

	class task_alive_token
	{
	public:
		struct Value { shared_ptr<Mutex> Sync; Value() : Sync(new Mutex) { } };
		TOOLKIT_DECLARE_PTR(Value);

	private:
		ValuePtr	_value;

	public:
		task_alive_token() : _value(new Value) { }
		~task_alive_token() { release(); }

		void release()
		{
			if (_value)
			{
				shared_ptr<Mutex> m = _value->Sync;
				MutexLock l(*m);
				_value.reset();
			}
		}

		ValueWeakPtr get_value() const { return _value;}
	};


}


#endif
