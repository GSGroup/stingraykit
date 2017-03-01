#ifndef STINGRAYKIT_THREAD_POSIX_POSIXTHREADSTATS_H
#define STINGRAYKIT_THREAD_POSIX_POSIXTHREADSTATS_H

// Copyright (c) 2011 - 2015, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/string/ToString.h>
#include <stingraykit/Holder.h>

#include <cstdio>
#include <string>

#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>


namespace stingray
{

	struct Stat
	{
		// 0
		s64				pid;
		char			tcomm[1024];
		char			state;
		s64				ppid;
		s64				pgid;
		// 5
		s64				sid;
		s64				tty_nr;
		s64				tty_pgrp;
		s64				flags;
		s64				min_flt;
		// 10
		s64				cmin_flt;
		s64				maj_flt;
		s64				cmaj_flt;
		s64				utime;
		s64				stimev;
		// 15
		s64				cutime;
		s64				cstime;
		s64				priority;
		s64				nicev;
		s64				num_threads;
		// 20
		s64				it_real_value;
		u64				start_time;
		s64				vsize;
		s64				rss;
		s64				rsslim;
		// 25
		s64				start_code;
		s64				end_code;
		s64				start_stack;
		s64				esp;
		s64				eip;
		// 30
		s64				pending;
		s64				blocked;
		s64				sigign;
		s64				sigcatch;
		s64				wchan;
		// 35
		s64				zero1;
		s64				zero2;
		s64				exit_signal;
		s64				processor;
		s64				rt_priority;
		// 40
		s64				policy;

		std::string ToString() const
		{
			string_ostream ss;
			ss		<< "pid: " << pid << ", tcomm: " << std::string(tcomm) << ", state: " << state << ", ppid: " << ppid << ", pgid: " << pgid
					<< ", sid: " << sid << ", tty_nr: " << tty_nr << ", tty_pgrp: " << tty_pgrp << ", flags: " << flags << ", min_flt: " << min_flt
					<< ", cmin_flt: " << cmin_flt << ", maj_flt: " << maj_flt << ", cmin_flt: " << cmaj_flt << ", utime: " << utime << ", stimev: " << stimev
					<< ", cutime: " << cutime << ", cstime: " << cstime << ", priority: " << priority << ", nicev: " << nicev << ", num_threads: " << num_threads
					<< ", it_real_value: " << it_real_value << ", start_time: " << start_time << ", vsize: " << vsize << ", rss: " << rss << ", rsslim: " << rsslim
					<< ", start_code: " << start_code << ", end_code: " << end_code << ", start_stack: " << start_stack << ", esp: " << esp << ", eip: " << eip
					<< ", pending: " << pending << ", blocked: " << blocked << ", sigign: " << sigign << ", sigcatch: " << sigcatch << ", wchan: " << wchan
					<< ", zero1: " << zero1 << ", zero2: " << zero2 << ", exit_signal: " << exit_signal << ", processor: " << processor << ", rt_priority: " << rt_priority
					<< ", policy: " << policy
			;
			return ss.str();
		}

		static bool GetThreadStats(u64 gid, u64 id, Stat& stat)
		{
#ifndef PLATFORM_STAPI
			std::string filename = StringBuilder() % "/proc/" % gid % "/task/" % id % "/stat";
#else
			// Looks like in ST linux correct CPU usage for thread 123 is accessible at /proc/123/task/123/stat
			std::string filename = StringBuilder() % "/proc/" % id % "/task/" % id % "/stat";
#endif

			int stat_f = open(filename.c_str(), O_RDONLY);
			if (stat_f != -1)
			{
				ScopedHolder<int> holder(stat_f, &close);
				return ReadCpuTimeFromStat(stat_f, stat);
			}
			else
			{
				filename = StringBuilder() % "/proc/" % id % "/stat";
				stat_f = open(filename.c_str(), O_RDONLY);
				if (stat_f == -1)
					return false;

				ScopedHolder<int> holder(stat_f, &close);
				return ReadCpuTimeFromStat(stat_f, stat);
			}
		}

	private:
		static optional<std::string> ConsumeFile(int fd)
		{
			static const size_t IncrementSize = 1024;
			std::string buffer(IncrementSize, 0);

			size_t read_total = 0;
			while (true)
			{
				const ssize_t ret = read(fd, &buffer[read_total], buffer.size() - read_total);
				if (ret < 0)
					return null;

				if (!ret)
					break;

				read_total += ret;
				if (read_total == buffer.size())
					buffer.resize(buffer.size() + IncrementSize);
			}

			buffer.resize(read_total);
			return buffer;
		}

		static bool ReadCpuTimeFromStat(int fd, Stat& s)
		{
			optional<std::string> result = ConsumeFile(fd);
			if (!result)
				return false;

			int count = sscanf(result->c_str(), "%lld (%1024[^)]) %c %lld %lld %lld %lld %lld %lld %lld %lld %lld %lld %lld %lld %lld %lld %lld %lld %lld %lld %lld %llu %lld %lld %lld %lld %lld %lld %lld %lld %lld %lld %lld %lld %lld %lld %lld %lld %lld %lld",
			// 0
			&s.pid,
			s.tcomm,
			&s.state,
			&s.ppid,
			&s.pgid,
			// 5
			&s.sid,
			&s.tty_nr,
			&s.tty_pgrp,
			&s.flags,
			&s.min_flt,
			// 10
			&s.cmin_flt,
			&s.maj_flt,
			&s.cmaj_flt,
			&s.utime,
			&s.stimev,
			// 15
			&s.cutime,
			&s.cstime,
			&s.priority,
			&s.nicev,
			&s.num_threads,
			// 20
			&s.it_real_value,
			&s.start_time,
			&s.vsize,
			&s.rss,
			&s.rsslim,
			// 25
			&s.start_code,
			&s.end_code,
			&s.start_stack,
			&s.esp,
			&s.eip,
			// 30
			&s.pending,
			&s.blocked,
			&s.sigign,
			&s.sigcatch,
			&s.wchan,
			// 35
			&s.zero1,
			&s.zero2,
			&s.exit_signal,
			&s.processor,
			&s.rt_priority,
			// 40
			&s.policy);

			static const int ArgsCount = 41;
			return count == ArgsCount;
		}
	};

}

#endif
