// Copyright (c) 2011 - 2022, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/thread/posix/Backtrace.h>

#include <stingraykit/string/Hex.h>
#include <stingraykit/Holder.h>

#ifdef HAVE_BFD_BACKTRACE
#	include <bfd.h>
#endif
#include <execinfo.h>

#include <iosfwd>
#include <sstream>

#define BACKTRACE_DEMANGLE 0

#if BACKTRACE_DEMANGLE
#	include <cxxabi.h>
#endif

#define BACKTRACE_FRAMES (64)

namespace stingray {
namespace posix
{

	static Mutex _backtrace_mutex;

	Backtrace::Backtrace()
	{
		_backtrace.resize(BACKTRACE_FRAMES);
		int frames = backtrace(&_backtrace[0], _backtrace.size());
		if (frames < 0)
			frames = 0;
		_backtrace.resize(frames);
	}

#ifdef HAVE_BFD_BACKTRACE
	struct bfd_holder {
		struct bfd *abfd;
		asymbol **symbols;
		bfd_boolean dynamic;


		bfd_holder(struct bfd *bfd = NULL): abfd(bfd), symbols(NULL) { bfd_init(); }
		~bfd_holder() { close(); }

		bool open(const char *fname)
		{
			close();
			abfd = bfd_openr(fname, NULL);
			if (!abfd)
				return false;
			if (!bfd_check_format (abfd, bfd_object))
			{
				bfd_close(abfd);
				abfd = NULL;
				return false;
			}
			return true;
		}

		void close()
		{
			if (symbols)
			{
				free(symbols);
				symbols = NULL;
			}
			if (abfd)
			{
				bfd_close(abfd);
				abfd = NULL;
			}
		}

		asymbol **get_symbols()
		{
			if (!symbols)
			{
				unsigned size;
				long count = bfd_read_minisymbols(abfd, dynamic, reinterpret_cast<void **>(&symbols), &size);
				if (count == 0)
				{
					dynamic = 1;
					count = bfd_read_minisymbols(abfd, dynamic, reinterpret_cast<void **>(&symbols), &size);
					if (count == 0)
					{
						return NULL;
					}
				}
			}
			return symbols;
		}

		bool find_in_section(asection *section, bfd_vma addr) const
		{
			if ((bfd_get_section_flags(abfd, section) & SEC_ALLOC) == 0)
				return false;

			bfd_vma vma = bfd_get_section_vma(abfd, section);
			if (addr < vma)
				return false;

			bfd_size_type size = bfd_section_size(abfd, section);
			return addr < vma + size;
		}
		operator struct bfd* () { return abfd; }
		struct bfd* operator ->() { return abfd; }
	} bfd;

	std::string Backtrace::Get() const
	{
		MutexLock l(_backtrace_mutex);
		if (!bfd.abfd)
		{
			if (!bfd.open("/proc/self/exe"))
				return "cannot open /proc/self/exe";
		}
		asymbol **symbols = bfd.get_symbols();
		if (!symbols)
			return "cannot load symbols";

		if (_backtrace.size() == 0)
			return "<empty>";

		string_ostream backtrace;
		try
		{
			for(size_t i = 0; i < _backtrace.size(); ++i)
			{
				bfd_vma addr = (bfd_vma)_backtrace[i];
				for(asection *section = bfd->sections; section != bfd->section_last; section = section->next)
				{
					if (bfd.find_in_section(section, addr))
					{
						bfd_vma vma = bfd_get_section_vma(bfd.abfd, section);
						const char *filename, *functionname;
						unsigned line;
						if (bfd_find_nearest_line(bfd.abfd, section, symbols, addr - vma, &filename, &functionname, &line))
						{
							backtrace << "0x" << ToHex(vma) << "\t" << filename << ":" << line << "\t";
#if BACKTRACE_DEMANGLE
							int status = 0;
							ScopedHolder<void*> holder(&free);
							char* buf = abi::__cxa_demangle(functionname, 0, 0, &status);
							if (buf)
								holder.Set(buf);
							if (buf && status == 0)
								backtrace << buf;
							else
								backtrace << functionname;
							backtrace << "\n";
#else
							backtrace << functionname << "\n";
#endif
						}
						else
						{
							//show only addr
							backtrace << "0x" << ToHex(vma) << "?" << "\n";
						}
					}
				}
			}
		} catch(const std::exception &e)
		{
			return e.what();
		}
		return backtrace.str();
	}

#else

	std::string Backtrace::Get() const
	{
		if (_backtrace.size() == 0)
			return "<empty>";

		string_ostream backtrace;

		for(size_t i = 0; i < _backtrace.size(); ++i)
		{
			if (i > 0)
				backtrace << " ";
			backtrace << ToHex((uintptr_t)_backtrace[i], sizeof(uintptr_t) * 2);
		}

		return backtrace.str();
	}

#endif

}}
