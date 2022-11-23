// Copyright (c) 2011 - 2022, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/diagnostics/SystemProfiler.h>

#include <stingraykit/log/Logger.h>

namespace stingray
{

	SystemProfiler::SystemProfiler(const std::string& message, s64 thresholdMs, s64 criticalMs)
		: _message(message), _thresholdMs(thresholdMs), _criticalMs(criticalMs)
	{ }


	SystemProfiler::~SystemProfiler()
	{
		try
		{
			const s64 elapsedMs = _elapsed.ElapsedMilliseconds();
			if (elapsedMs >= _thresholdMs)
			{
				Logger::Stream(_criticalMs >= _thresholdMs && elapsedMs > _criticalMs ? LogLevel::Warning : LogLevel::Info)
						<< _message << ": " << elapsedMs << " ms";
			}
		}
		catch (const std::exception&)
		{ }
	}

}
