// Copyright (c) 2011 - 2017, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/io/BufferedPipe.h>
#include <stingraykit/thread/TimedCancellationToken.h>

namespace stingray
{

	BufferedPipe::BufferedPipe(const IPipePtr& pipe, size_t bufferSize)
		: _pipe(pipe), _buffer(bufferSize), _bufferOffset(), _bufferSize()
	{ }


	u64 BufferedPipe::Read(ByteData data, const ICancellationToken& token, const optional<TimeDuration>& timeout)
	{
		if (_bufferOffset == _bufferSize)
		{
			const size_t size = _pipe->Read(_buffer.GetByteData(), timeout ? const_ref(TimedCancellationToken(token, *timeout)) : token);
			if (size == 0)
				return 0;

			_bufferOffset = 0;
			_bufferSize = size;
		}

		const size_t size = std::min(_bufferSize - _bufferOffset, data.size());
		std::copy(_buffer.data() + _bufferOffset, _buffer.data() + _bufferOffset + size, data.data());
		_bufferOffset += size;

		return size;
	}


	u64 BufferedPipe::Write(ConstByteData data, const ICancellationToken& token, const optional<TimeDuration>& timeout)
	{ return _pipe->Write(data, timeout ? const_ref(TimedCancellationToken(token, *timeout)) : token); }

}
