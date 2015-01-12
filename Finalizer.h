#ifndef STINGRAY_STINGRAYKIT_FINALIZER_H
#define STINGRAY_STINGRAYKIT_FINALIZER_H


#include <stingray/toolkit/thread/ITaskExecutor.h>
#include <stingray/toolkit/function/bind.h>

namespace stingray
{

	struct Finalizer
	{
	private:
		ITaskExecutorPtr	_executor;

	public:
		Finalizer(const ITaskExecutorPtr& executor) : _executor(STINGRAYKIT_REQUIRE_NOT_NULL(executor))
		{ }

		template< typename T >
		void operator () (T* t)
		{ _executor->AddTask(bind(&Finalizer::DoDelete<T>, t)); }

	private:
		template< typename T >
		static void DoDelete(T* t)
		{ delete t; }
	};

}

#endif
