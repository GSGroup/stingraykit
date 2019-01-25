#ifndef STINGRAYKIT_THREAD_CALL_ONCE_H
#define STINGRAYKIT_THREAD_CALL_ONCE_H

// Copyright (c) 2011 - 2019, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#if PLATFORM_POSIX
#	include <stingraykit/thread/posix/PosixCallOnce.h>
	namespace stingray { typedef PosixCallOnce	CallOnce; }
#else
#	error Threads not configured
#endif

namespace stingray
{

	/**
	 * @addtogroup toolkit_threads
	 * @{
	 */

#define STINGRAYKIT_DECLARE_ONCE_FLAG(flagName_) ::stingray::CallOnce::OnceNativeType flagName_
#define STINGRAYKIT_DEFINE_ONCE_FLAG(flagName_) ::stingray::CallOnce::OnceNativeType flagName_ = STINGRAYKIT_ONCE_INIT_VALUE

	inline void call_once(CallOnce::OnceNativeType& flag, void (*func)())
	{ CallOnce::CallOnce(flag, func); }

	/** @} */

}


#endif
