#include <stingraykit/io/PipeReader.h>

namespace stingray
{

	PipeReader::PipeReader(const IPipePtr& pipe)
		: _pipe(pipe)
	{ }


	u8 PipeReader::ReadByte(const ICancellationToken& token)
	{
		u8 result;
		STINGRAYKIT_CHECK(Read(ByteData(&result, sizeof(result)), token) == sizeof(result), OperationCanceledException());
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
			return result;
		}
	}

}
