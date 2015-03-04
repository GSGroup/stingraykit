#ifndef STINGRAYKIT_THREAD_OSPLUS_OS21TASKEXECUTOR_H
#define STINGRAYKIT_THREAD_OSPLUS_OS21TASKEXECUTOR_H

// Copyright (c) 2011 - 2015, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <os21/message.h>

#include <string>

#include <stingraykit/thread/Thread.h>
#include <stingraykit/Final.h>
#include <stingraykit/thread/ITaskExecutor.h>


namespace stingray {
namespace osplus
{


	class TaskExecutor
		: STINGRAYKIT_FINAL(TaskExecutor)
		, public virtual ITaskExecutor
	{
		typedef function<void()>							TaskType;
		typedef function<void(const std::exception&)>		ExceptionHandlerType;
		typedef std::pair<TaskType, FutureExecutionTester>	TaskPair;

	private:
		ThreadPtr				_worker;
		ExceptionHandlerType	_exceptionHandler;
		message_queue_t*		_queue;

	public:
		explicit TaskExecutor(const std::string& name, const ExceptionHandlerType& exceptionHandler);
		~TaskExecutor();

		virtual void AddTask(const TaskType& task);
		virtual void AddTask(const TaskType& task, const FutureExecutionTester& tester);

	private:
		void ThreadFunc();
	};


}}


#endif
