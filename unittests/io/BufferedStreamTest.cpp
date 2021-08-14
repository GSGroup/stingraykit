#include <stingraykit/io/BufferedStream.h>
#include <stingraykit/io/MemoryByteStream.h>
#include <stingraykit/Random.h>

#include <gtest/gtest.h>

using namespace stingray;

class BufferedStreamTest : public testing::Test
{
protected:
	static const u64 DataSize 	= 256;
	static const u64 BufferSize	= 16;
	static const u64 Alignment	= 4;

	static const u64 AssetSize	= 32;

	static const u32 Attempts	= 1000;

protected:
	const ByteArray			_data;
	const IByteStreamPtr	_rs;
	const IByteStreamPtr	_bs;
	Random					_random;

	BufferedStreamTest() :
		_data(DataSize),
		_rs(CreateMemoryByteStream(_data)),
		_bs(new BufferedStream(_rs, BufferSize, Alignment))
	{ }

protected:
	void Fill(ByteData data)
	{
		for (size_t i = 0; i < data.size(); ++i)
			data[i] = _random.Next(std::numeric_limits<ByteArray::value_type>::max());
	}

	void CheckBuffer(ByteData data, u64 offset)
	{
		for (size_t i = 0; i < data.size(); ++i)
			ASSERT_EQ(data[i], _data[offset + i]);
	}

	void CheckStreams(u64 offset)
	{
		ASSERT_EQ(_bs->Tell(), offset);
		ASSERT_EQ(_rs->Tell() % Alignment, 0u);
	}

	void Seek(s64 offset, SeekMode mode)
	{
		u64 ro = 0;
		switch (mode)
		{
		case SeekMode::Begin:	ro = offset;				break;
		case SeekMode::Current:	ro = offset + _bs->Tell();	break;
		case SeekMode::End:		ro = offset + DataSize;		break;
		}

		_bs->Seek(offset, mode);
		CheckStreams(ro);
	}

	u64 Read(u64 offset, u64 count)
	{
		const ByteArray buffer(count);
		const u64 n = _bs->Read(buffer, DummyCancellationToken());
		CheckBuffer(ByteData(buffer, 0, n), offset);
		CheckStreams(offset + n);
		return n;
	}

	u64 Write(u64 offset, u64 count)
	{
		const ByteArray buffer(count);
		Fill(buffer);
		const u64 n = _bs->Write(buffer, DummyCancellationToken());
		CheckBuffer(buffer, offset);
		CheckStreams(offset + n);
		return n;
	}
};


TEST_F(BufferedStreamTest, Read)
{
	Fill(_data);
	for (size_t i = 0; i < Attempts; ++i)
	{
		const u64 count = _random.Next(AssetSize);
		const u64 offset = _random.Next(DataSize);

		Seek(offset, SeekMode::Begin);
		const u64 n = std::min<u64>(count, DataSize - offset);
		ASSERT_EQ(Read(offset, count), n);
	}
}


TEST_F(BufferedStreamTest, Write)
{
	for (size_t i = 0; i < Attempts; ++i)
	{
		const u64 count = _random.Next(AssetSize);
		const u64 offset = _random.Next(DataSize);

		Seek(offset, SeekMode::Begin);
		const u64 n = std::min<u64>(count, DataSize - offset);
		ASSERT_EQ(Write(offset, n), n);
	}
}


TEST_F(BufferedStreamTest, Seek)
{
	Fill(_data);
	for (size_t i = 0; i < Attempts; ++i)
	{
		const SeekMode mode(SeekMode::Enum(i % (SeekMode::end() - SeekMode::begin())));
		const s64 offset = _random.Next(DataSize);
		const u64 count = std::min<u64>(1, DataSize - offset);

		switch (mode)
		{
		case SeekMode::Begin:	Seek(offset, mode);					break;
		case SeekMode::Current:	Seek(offset - _bs->Tell(), mode);	break;
		case SeekMode::End:		Seek(offset - DataSize, mode);		break;
		}
		ASSERT_EQ(Read(offset, 1), count);
	}
}
