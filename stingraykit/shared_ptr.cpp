// Copyright (c) 2011 - 2019, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/shared_ptr.h>

#include <stingraykit/diagnostics/Backtrace.h>
#include <stingraykit/log/Logger.h>
#include <stingraykit/string/Hex.h>


namespace stingray
{
	namespace Detail
	{
		void DoLogAddRef(const char* className, u32 refs, const void* objPtrVal, const void* sharedPtrPtrVal)
		{ Logger::Warning() << "[shared_ptr] +++ Addrefed " << className << " (obj: 0x" << Hex(objPtrVal, 8) << ", shared_ptr: 0x" << Hex(sharedPtrPtrVal, 8) << "), new value = " << refs << "\n" << Backtrace().Get(); }

		void DoLogReleaseRef(const char* className, u32 refs, const void* objPtrVal, const void* sharedPtrPtrVal)
		{ Logger::Warning() << "[shared_ptr] --- Released " << className << " (obj: 0x" << Hex(objPtrVal, 8) << ", shared_ptr: 0x" << Hex(sharedPtrPtrVal, 8) << "), new value = " << refs << "\n" << Backtrace().Get(); }
	}
}
