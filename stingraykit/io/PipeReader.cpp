// Copyright (c) 2011 - 2022, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/io/PipeReader.h>

namespace stingray
{

	PipeReader::PipeReader(const IPipePtr& pipe)
		: _pipe(STINGRAYKIT_REQUIRE_NOT_NULL(pipe))
	{ }


	u8 PipeReader::ReadByte(const ICancellationToken& token)
	{
		u8 result;
		if (Read(ByteData(&result, sizeof(result)), token) != sizeof(result))
			STINGRAYKIT_ASSUME_CANCELLATION(token);
		return result;
	}


	std::string PipeReader::ReadLine(const ICancellationToken& token)
	{
		std::string result;

		try
		{
			for (u8 byte = ReadByte(token); ; byte = ReadByte(token))
			{
				if (byte == '\n')
					return result;

				if (byte == '\r')
				{
					const u8 next = ReadByte(token);
					if (next == '\n')
						return result;
					STINGRAYKIT_THROW(NotSupportedException());
				}

				result.push_back(byte);
			}
		}
		catch (const PipeClosedException&)
		{
			if (result.empty())
				throw;
		}

		return result;
	}

}
