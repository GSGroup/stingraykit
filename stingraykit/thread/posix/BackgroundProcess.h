#ifndef STINGRAYKIT_THREAD_POSIX_BACKGROUNDPROCESS_H
#define STINGRAYKIT_THREAD_POSIX_BACKGROUNDPROCESS_H

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
		void Monitor();
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
