// Copyright (c) 2011 - 2017, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <memory>

#include <stingraykit/thread/osplus/OS21TaskExecutor.h>
#include <stingraykit/function/bind.h>


namespace stingray {
namespace osplus
{


	TaskExecutor::TaskExecutor(const std::string& name, const ExceptionHandlerType& exceptionHandler)
		: _exceptionHandler(exceptionHandler)
	{
		_queue = message_create_queue(sizeof(TaskPair*), 128); /* TODO: replace with message_create_queue_p */
		STINGRAYKIT_CHECK(_queue, Exception("message_create_queue"));
		_worker.reset(new Thread(name, bind(&TaskExecutor::ThreadFunc, this, not_using(_1))));
	}

	TaskExecutor::~TaskExecutor()
	{
		TaskPair** msg = reinterpret_cast<TaskPair**>(message_claim(_queue));
		*msg = 0;
		message_send(_queue, msg);
		_worker.reset();
		message_delete_queue(_queue);
	}

	void TaskExecutor::AddTask(const TaskType& task, const FutureExecutionTester& tester)
	{
		TaskPair** msg = reinterpret_cast<TaskPair**>(message_claim(_queue));
		*msg = new TaskPair(std::make_pair(task, tester));
		message_send(_queue, msg);
	}

	void TaskExecutor::AddTask(const TaskType& task) { AddTask(task, null); }

	void TaskExecutor::ThreadFunc()
	{
		TaskPair** msg;
		while ((msg = reinterpret_cast<TaskPair**>(message_receive(_queue))) != 0 && *msg != 0)
		{
			std::auto_ptr<TaskPair> task_pair(*msg);
			message_release(_queue, msg);
			try
			{
				LocalExecutionGuard exec_guard(task_pair->second);
				if (exec_guard)
					task_pair->first();
				Thread::InterruptionPoint();
			}
			catch(const std::exception& ex)
			{
				_exceptionHandler(ex);
			}
		}
	}


}}
