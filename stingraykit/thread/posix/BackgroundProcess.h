#ifndef STINGRAYKIT_THREAD_POSIX_BACKGROUNDPROCESS_H
#define STINGRAYKIT_THREAD_POSIX_BACKGROUNDPROCESS_H

// Copyright (c) 2011 - 2018, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/thread/Thread.h>
#include <stingraykit/log/Logger.h>
#include <sys/types.h>

namespace stingray { namespace posix
{

	class BackgroundProcess
	{
		static NamedLogger			s_logger;

	private:
		ThreadPtr					_monitor;

		typedef std::vector<char>	ArgHolder;

		std::string					_name;
		std::vector<ArgHolder>		_args;
		bool						_restart;

		std::vector<char *>			_argv;
		pid_t						_pid;

	private:
		void Convert(ArgHolder &dst, const std::string &src);
		void Monitor(const ICancellationToken& token);
		void Start();
		void Stop();
		void Wait();

	public:
		BackgroundProcess(const std::string &name, const std::vector<std::string> &args, bool restart);
		~BackgroundProcess();

		inline int GetPid() const { return _pid; }
		void Kill(int sig);
	};
	STINGRAYKIT_DECLARE_PTR(BackgroundProcess);

}}


#endif
