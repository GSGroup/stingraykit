#ifndef STINGRAYKIT_THREAD_THREADOPERATION_H
#define STINGRAYKIT_THREAD_THREADOPERATION_H


#include <stingraykit/log/Logger.h>
#include <stingraykit/toolkit.h>


namespace stingray
{

	struct ThreadOperation
	{
		STINGRAYKIT_ENUM_VALUES(
			Network = 1,
			IO = 2);

		STINGRAYKIT_DECLARE_ENUM_CLASS(ThreadOperation);
	};
	STINGRAYKIT_DECLARE_ENUM_CLASS_BIT_OPERATORS(ThreadOperation);


	class ThreadOperationConstrainer
	{
	private:
		int	_oldValue;

	public:
		ThreadOperationConstrainer(ThreadOperation restrictedOperations);
		~ThreadOperationConstrainer();
	};


	class ThreadOperationReporter
	{
	private:
		static NamedLogger	s_logger;

	public:
		ThreadOperationReporter(ThreadOperation op);
		~ThreadOperationReporter();
	};

}

#endif
