#ifndef STINGRAYKIT_THREAD_THREADOPERATION_H
#define STINGRAYKIT_THREAD_THREADOPERATION_H

// Copyright (c) 2011 - 2024, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/log/Logger.h>

namespace stingray
{

	struct ThreadOperation
	{
		STINGRAYKIT_ENUM_VALUES
		(
			Network = 1,
			IO = 2,
			UI = 4
		);

		STINGRAYKIT_DECLARE_ENUM_CLASS(ThreadOperation);
	};
	STINGRAYKIT_DECLARE_ENUM_CLASS_BIT_OPERATORS(ThreadOperation);


	class ThreadOperationConstrainer
	{
		STINGRAYKIT_NONCOPYABLE(ThreadOperationConstrainer);

	private:
		int						_oldValue;

	public:
		ThreadOperationConstrainer(ThreadOperation restrictedOperations);
		~ThreadOperationConstrainer();
	};


	class ThreadOperationReporter
	{
		STINGRAYKIT_NONCOPYABLE(ThreadOperationReporter);

	private:
		static NamedLogger		s_logger;

	public:
		ThreadOperationReporter(ThreadOperation op);
	};


	class ExclusiveThreadOperation
	{
		STINGRAYKIT_NONCOPYABLE(ExclusiveThreadOperation);

	private:
		int						_oldValue;

	public:
		ExclusiveThreadOperation(ThreadOperation op);
		~ExclusiveThreadOperation();
	};


	class ExclusiveThreadOperationChecker
	{
		STINGRAYKIT_NONCOPYABLE(ExclusiveThreadOperationChecker);

	private:
		static NamedLogger		s_logger;

	public:
		ExclusiveThreadOperationChecker(ThreadOperation op);
	};

}

#endif
