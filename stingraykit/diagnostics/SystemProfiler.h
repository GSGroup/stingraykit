#ifndef STINGRAYKIT_DIAGNOSTICS_SYSTEMPROFILER_H
#define STINGRAYKIT_DIAGNOSTICS_SYSTEMPROFILER_H

// Copyright (c) 2011 - 2019, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.


#include <memory>
#include <string>
#include <stingraykit/toolkit.h>
#include <stingraykit/unique_ptr.h>


namespace stingray
{

	/**
	 * @addtogroup toolkit_threads
	 * @{
	 */

	class SystemProfiler
	{
		STINGRAYKIT_NONCOPYABLE(SystemProfiler);

		class Impl;

		unique_ptr<Impl>		_impl;
		std::string				_message;
		s64						_thresholdMs;
		s64						_criticalMs;
		s64						_start;

	public:
		SystemProfiler(const std::string& message, s64 thresholdMs, s64 criticalMs = 0);
		~SystemProfiler();
	};

	/** @} */

}


#endif
