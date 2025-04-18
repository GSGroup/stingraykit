// Copyright (c) 2011 - 2025, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/diagnostics/gcc/Backtrace.h>

#include <stingraykit/string/Hex.h>

#include <unwind.h>

namespace stingray {
namespace gcc
{

#ifdef __clang__
	typedef unsigned long _Unwind_Word;
	typedef uintptr_t _Unwind_Ptr;

#	if __clang_major__ < 3 && __clang_minor__ < 4
		static inline _Unwind_Word _Unwind_GetCFA(struct _Unwind_Context *) { return 0; }
#	endif
#endif

	static const unsigned EmptyBacktraceEntry = ~0u;

	namespace
	{
		class BacktraceImpl
		{
			Backtrace::BacktraceArray&	_backtrace;
			unsigned&					_size;
			_Unwind_Word				_cfa;

			_Unwind_Reason_Code UnwindFrame(struct _Unwind_Context *ctx)
			{
				_Unwind_Ptr ip = _Unwind_GetIP(ctx);
				_Unwind_Word cfa = _Unwind_GetCFA(ctx);
				if (_size != 0 && _backtrace[_size - 1] == ip && cfa == _cfa)
					return _URC_END_OF_STACK;

				_cfa = cfa;
				_backtrace[_size++] = ip;
				return (_size >= Backtrace::MaxFrames)? _URC_END_OF_STACK: _URC_NO_REASON;
			}

			static _Unwind_Reason_Code UnwindFunc(struct _Unwind_Context *ctx, void *bt)
			{
				BacktraceImpl *self = static_cast<BacktraceImpl *>(bt);
				return self->UnwindFrame(ctx);
			}

		public:
			BacktraceImpl(Backtrace::BacktraceArray& backtrace, unsigned& size): _backtrace(backtrace), _size(size), _cfa()
			{ }

			void Unwind()
			{ _Unwind_Backtrace(&UnwindFunc, this); }
		};
	}


	Backtrace::Backtrace() : _size(0)
	{
		BacktraceImpl impl(_backtrace, _size);
		impl.Unwind();
	}


	std::string Backtrace::Get() const
	{
		if (_size == 0)
			return "<empty>";

		string_ostream backtrace;

		for (size_t i = 0; i < _size; ++i)
		{
			if (i > 0)
				backtrace << " ";
			backtrace << ToHex(_backtrace[i], sizeof(uintptr_t) * 2);
		}

		return backtrace.str();
	}

}}
