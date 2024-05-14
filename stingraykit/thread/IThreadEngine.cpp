// Copyright (c) 2011 - 2024, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/thread/IThreadEngine.h>

#include <stingraykit/string/ToString.h>

namespace stingray
{

	std::string ThreadSchedulingParams::ToString() const
	{ return StringBuilder() % "{ policy: " % _policy % ", priority: " % _priority % " }"; }


	std::string ThreadCpuStats::ToString() const
	{ return StringBuilder() % "{ user: " % _uTime.GetMilliseconds() % " ms, system: " % _sTime.GetMilliseconds() % " ms }"; }


	std::string ThreadStats::ToString() const
	{ return StringBuilder() % "{ tid: " % _threadId % ", parent: " % _parentId % ", name: " % _threadName % ", stats: " % _cpuStats % ", children: " % _childrenCpuStats % " }"; }


	std::string SystemStats::ToString() const
	{ return StringBuilder() % "{ user: " % _user % " ticks, system: " % _system % " ticks, iowait: " % _iowait % " ticks, idle: " % _idle % " }"; }

}
