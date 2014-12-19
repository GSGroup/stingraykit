#ifndef STINGRAY_TOOLKIT_THREAD_POSIX_SUBPROCESS_H
#define STINGRAY_TOOLKIT_THREAD_POSIX_SUBPROCESS_H


#include <stingray/toolkit/log/Logger.h>
#include <stingray/toolkit/thread/Thread.h>
#include <stingray/toolkit/io/IByteStream.h>
#include <stingray/toolkit/optional.h>
#include <stingray/platform/posix/filesystem/Pipe.h>

#include <string>
#include <vector>

#include <signal.h>


namespace stingray {
namespace posix
{

	struct OutputForwardMode
	{
		TOOLKIT_ENUM_VALUES
		(
			None			= 0x00,
			Stdout			= 0x01,
			Stderr			= 0x02,
			StderrToStdout	= 0x04
		);
		TOOLKIT_DECLARE_ENUM_CLASS(OutputForwardMode);
	};
	TOOLKIT_DECLARE_ENUM_CLASS_BIT_OPERATORS(OutputForwardMode);


	class Subprocess
	{
		static NamedLogger	s_logger;

	private:
		pid_t			_pid;
		PipePtr			_stdoutPipe;
		PipePtr			_stderrPipe;
		optional<int>	_exitCode;
		Mutex			_sync;

	public:
		Subprocess(const std::string& name, const std::vector<std::string>& args, OutputForwardMode fwMode = OutputForwardMode::None);
		Subprocess(const std::string& execLine, OutputForwardMode fwMode = OutputForwardMode::None);
		~Subprocess();

		bool Poll() const;
		void Wait();
		void Kill(int sig = SIGKILL);

		int GetReturnCode() const;
		IByteStreamPtr GetStdout() const;
		IByteStreamPtr GetStderr() const;

	private:
		void Start(std::string& name, std::vector<char*>& argv, OutputForwardMode fwMode);
	};
	TOOLKIT_DECLARE_PTR(Subprocess);

}}


#endif
