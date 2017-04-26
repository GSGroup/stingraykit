// Copyright (c) 2011 - 2017, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/thread/ThreadOperation.h>

#include <stingraykit/diagnostics/Backtrace.h>
#include <stingraykit/thread/Thread.h>
#include <stingraykit/thread/posix/ThreadLocal.h>

namespace stingray
{

	STINGRAYKIT_DECLARE_THREAD_LOCAL(int, RestrictedThreadOperations);
	STINGRAYKIT_DEFINE_THREAD_LOCAL(int, RestrictedThreadOperations);

	ThreadOperationConstrainer::ThreadOperationConstrainer(ThreadOperation restrictedOperations) :
		_oldValue(RestrictedThreadOperations::Get())
	{ RestrictedThreadOperations::Get() = _oldValue | restrictedOperations.val(); }


	ThreadOperationConstrainer::~ThreadOperationConstrainer()
	{ RestrictedThreadOperations::Get() = _oldValue; }


	STINGRAYKIT_DEFINE_NAMED_LOGGER(ThreadOperationReporter);

	ThreadOperationReporter::ThreadOperationReporter(ThreadOperation op)
	{
		if (op.val() & RestrictedThreadOperations::Get())
			s_logger.Error() << op << " operations are prohibited in this thread!\nBacktrace: " << Backtrace();
	}

	ThreadOperationReporter::~ThreadOperationReporter()
	{ }


	STINGRAYKIT_DECLARE_THREAD_LOCAL(int, ExclusiveThreadOperations);
	STINGRAYKIT_DEFINE_THREAD_LOCAL(int, ExclusiveThreadOperations);

	ExclusiveThreadOperation::ExclusiveThreadOperation(ThreadOperation op):
		_oldValue(ExclusiveThreadOperations::Get())
	{
		ExclusiveThreadOperations::Get() = _oldValue | op.val();
	}

	ExclusiveThreadOperation::~ExclusiveThreadOperation()
	{
		ExclusiveThreadOperations::Get() = _oldValue;
	}

	STINGRAYKIT_DEFINE_NAMED_LOGGER(ExclusiveThreadOperationChecker);

	ExclusiveThreadOperationChecker::ExclusiveThreadOperationChecker(ThreadOperation op)
	{
		if (!(op.val() & ExclusiveThreadOperations::Get()))
		{
			s_logger.Error() << op << " operations happened out of exclusive operation area: " << Backtrace();
#ifndef PLATFORM_EMBEDDED
			STINGRAYKIT_FATAL("forbidden exclusive operation");
#endif
		}
	}

	ExclusiveThreadOperationChecker::~ExclusiveThreadOperationChecker()
	{ }

}
