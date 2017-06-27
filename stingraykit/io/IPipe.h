#ifndef STINGRAYKIT_IO_IPIPE_H
#define STINGRAYKIT_IO_IPIPE_H

// Copyright (c) 2011 - 2017, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/io/IInputByteStream.h>
#include <stingraykit/io/IOutputByteStream.h>
#include <stingraykit/optional.h>
#include <stingraykit/time/Time.h>

namespace stingray
{

	STINGRAYKIT_DECLARE_SIMPLE_EXCEPTION(PipeClosedException, "Pipe has been closed!");


	struct IPipe : public virtual IInputByteStream, public virtual IOutputByteStream
	{
		virtual u64 Read(ByteData data, const ICancellationToken& token) { return Read(data, token, null); }
		virtual u64 Read(ByteData data, const ICancellationToken& token, const optional<TimeDuration>& timeout) = 0;

		virtual u64 Write(ConstByteData data, const ICancellationToken& token) { return Write(data, token, null); }
		virtual u64 Write(ConstByteData data, const ICancellationToken& token, const optional<TimeDuration>& timeout) = 0;
	};
	STINGRAYKIT_DECLARE_PTR(IPipe);

}

#endif
