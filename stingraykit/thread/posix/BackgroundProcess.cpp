// Copyright (c) 2011 - 2015, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/thread/posix/BackgroundProcess.h>

#include <stingraykit/SystemException.h>
#include <stingraykit/function/bind.h>

#include <stdio.h>
#include <stdlib.h>

#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#if HAVE_POSIX_SPAWN
#	include <spawn.h>
#endif


namespace stingray { namespace posix
{
	STINGRAYKIT_DEFINE_NAMED_LOGGER(BackgroundProcess);

	namespace
	{
		static const unsigned RestartInterval = 3000;
	}

	BackgroundProcess::BackgroundProcess(const std::string &name, const std::vector<std::string> &args, bool restart) :
		_name(name), _restart(restart), _pid(-1)
	{
#if defined(STINGRAY_USE_VFORK_DISABLED)
		STINGRAYKIT_THROW("the use of vfork are prohibited!");
#endif
		std::vector<std::string> args_copy;
		args_copy.push_back(name);
		std::copy(args.begin(), args.end(), std::back_inserter(args_copy));

		_args.resize(args_copy.size());
		for(size_t i = 0; i < args_copy.size(); ++i)
			Convert(_args[i], args_copy[i]);

		_argv.push_back(0);

		_monitor = make_shared<Thread>("monitor: " + name, bind(&BackgroundProcess::Monitor, this, _1));
	}

	void BackgroundProcess::Convert(ArgHolder &dst, const std::string &src)
	{
		std::copy(src.begin(), src.end(), std::back_inserter(dst));
		dst.push_back(0);

		_argv.push_back(&dst[0]);
	}

	void BackgroundProcess::Monitor(const ICancellationToken& token)
	{
		while(token && _restart)
		{
			STINGRAYKIT_TRY("starting process failed ", Start());
			Wait();
			s_logger.Info() << "process " << _name << " exited";;
			Thread::Sleep(RestartInterval);
			s_logger.Info() << "restarting " << _name;
		}
	}

	void BackgroundProcess::Start()
	{
		Stop();

		s_logger.Info() << "starting: " << _name;

		const char *file = _name.c_str();
#if HAVE_POSIX_SPAWN
		int status = posix_spawnp(&_pid, file, NULL, NULL, &_argv[0], environ);
		if (status != 0)
			STINGRAYKIT_THROW(SystemException("posix_spawnp failed"));
#else
		pid_t pid = vfork();
		//no c++ code below this point
		if (pid == 0)
		{
			if (execvp(file, &_argv[0]) == -1)
				perror("execvp");

			exit(-1);
		}
		else
			_pid = pid;
#endif
	}

	void BackgroundProcess::Wait()
	{
		if (_pid > 0)
		{
			int r = waitpid(_pid, NULL, 0);
			if (r == -1)
				perror("waitpid");
		}
	}

	void BackgroundProcess::Stop()
	{
		if (_pid > 0)
		{
			s_logger.Info() << "stopping: " << _name;
			if (kill(_pid, SIGTERM) == -1)
			{
				perror("kill(SIGTERM)");
				return;
			}
			Thread::Sleep(2000);
			if (kill(_pid, SIGKILL) == -1)
			{
				perror("kill(SIGKILL)");
			}
			waitpid(_pid, NULL, 0); //kill zombie
			_pid = -1;
		}
	}

	void BackgroundProcess::Kill(int sig)
	{
		if (_pid > 0)
		{
			if (kill(_pid, sig) == -1)
				perror("kill");
		}
	}

	BackgroundProcess::~BackgroundProcess()
	{
		_monitor->Interrupt();
		_monitor.reset();
		Stop();
	}

}}
