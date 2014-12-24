#include <stingray/toolkit/thread/posix/Subprocess.h>

#include <stingray/toolkit/exception.h>
#include <stingray/toolkit/string/StringUtils.h>
#include <stingray/toolkit/SystemException.h>

#include <unistd.h>

#include <sys/types.h>
#include <sys/wait.h>


namespace stingray {
namespace posix
{

	TOOLKIT_DEFINE_NAMED_LOGGER(Subprocess);


	Subprocess::Subprocess(const std::string& name, const std::vector<std::string>& args, OutputForwardMode fwMode) : _pid(-1)
	{
		std::string name_copy(name);
		std::vector<std::string> args_copy(args);

		std::vector<char*> argv(args_copy.size() + 2);
		argv[0] = &name_copy[0];
		for (size_t i = 0; i < args_copy.size(); ++i)
			argv[i + 1] = &args_copy[i][0];

		Start(name_copy, argv, fwMode);
	}

	Subprocess::Subprocess(const std::string& execLine, OutputForwardMode fwMode) : _pid(-1)
	{
		std::vector<std::string> args;
		Split(execLine, " ", args);

		std::vector<char*> argv(args.size() + 1);
		for (size_t i = 0; i < args.size(); ++i)
			argv[i] = &args[i][0];

		Start(args[0], argv, fwMode);
	}

	void Subprocess::Start(std::string& name, std::vector<char*>& argv, OutputForwardMode fwMode)
	{
#if defined(STINGRAY_USE_VFORK_DISABLED)
		TOOLKIT_THROW("the use of vfork are prohibited!");
#endif
		if (fwMode & OutputForwardMode::Stdout)
			_stdoutPipe.reset(new Pipe);
		if ((fwMode & OutputForwardMode::Stderr) && !(fwMode & OutputForwardMode::StderrToStdout))
			_stderrPipe.reset(new Pipe);

		pid_t pid = vfork();
		TOOLKIT_CHECK(pid != -1, SystemException("vfork"));

		if (pid == 0)
		{
			try
			{
				if (_stdoutPipe)
				{
					TOOLKIT_CHECK(dup2(_stdoutPipe->GetWriteStream()->Get(), 1) == 1, Exception("stdout dup2"));
					if (fwMode & OutputForwardMode::StderrToStdout)
						TOOLKIT_CHECK(dup2(_stdoutPipe->GetWriteStream()->Get(), 2) == 2, Exception("stderr to stdout dup2"));
				}
				if (_stderrPipe && !(fwMode & OutputForwardMode::StderrToStdout))
					TOOLKIT_CHECK(dup2(_stderrPipe->GetWriteStream()->Get(), 2) == 2, Exception("stderr dup2"));
			}
			catch (const std::runtime_error& ex)
			{ perror(ex.what()); }

			execvp(name.c_str(), argv.data());
			perror("execvp");
			exit(-1);
		}
		else
		{
			_pid = pid;
		}
	}

	Subprocess::~Subprocess()
	{
		try { Wait(); } catch (const SystemException&) { }
	}

	bool Subprocess::Poll() const
	{
		int result = waitpid(_pid, NULL, WNOHANG);
		TOOLKIT_CHECK(result != -1, SystemException("waitpid"));
		return !result;
	}

	void Subprocess::Wait()
	{
		MutexLock l(_sync);

		if (_exitCode.is_initialized())
			return;

		int status;
		int result = waitpid(_pid, &status, 0);
		TOOLKIT_CHECK(result == _pid, SystemException("waitpid"));

		_exitCode = WIFEXITED(status) ? WEXITSTATUS(status) : -1; /* return -1 if subprocess was terminated by a signal, WIFSTOPPED "not handled" */
	}

	void Subprocess::Kill(int sig)
	{
		TOOLKIT_CHECK(kill(_pid, sig) == 0, SystemException("kill"));
	}

	int Subprocess::GetReturnCode() const
	{
		TOOLKIT_CHECK(_exitCode.is_initialized(), Exception("still running or zombie"));
		return _exitCode.get();
	}

	IByteStreamPtr Subprocess::GetStdout() const
	{ return make_shared<FileByteStream>(_stdoutPipe->GetReadStream()); }

	IByteStreamPtr Subprocess::GetStderr() const
	{ return make_shared<FileByteStream>(_stderrPipe->GetReadStream()); }

}}
