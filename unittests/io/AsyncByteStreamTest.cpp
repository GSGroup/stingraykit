#include <stingraykit/collection/ByteData.h>
#include <stingraykit/io/AsyncByteStream.h>
#include <stingraykit/io/MemoryByteStream.h>
#include <stingraykit/log/Logger.h>
#include <stingraykit/Random.h>

#include <gtest/gtest.h>

using namespace stingray;

namespace
{

	template < typename ContainerType_ >
	class DelayedMemoryByteStream : public virtual MemoryByteStream<ContainerType_>
	{
		typedef MemoryByteStream<ContainerType_> base;

		const TimeDuration		_delay;

	public:
		DelayedMemoryByteStream(const ContainerType_& data, TimeDuration delay) :
			base(data),
			_delay(delay)
		{ }

		virtual u64 Read(ByteData data, const ICancellationToken& token)
		{
			token.Sleep(_delay);
			return base::Read(data, token);
		}

		virtual u64 Write(ConstByteData data, const ICancellationToken& token)
		{
			token.Sleep(_delay);
			return base::Write(data, token);
		}
	};

	ByteArray GenerateRandomArray(size_t size)
	{
		Logger::Info() << "Generating random data...";

		ByteArray randomData(size);
		Random _random;

		size_t idx;
		for (idx = 0; idx < (randomData.size() - (randomData.size() % sizeof(u32))); idx += sizeof(u32))
			*((u32 *)&randomData[idx]) = _random.Next();
		for ( ; idx < randomData.size(); idx += sizeof(u8))
			randomData[idx] = (u8)_random.Next();

		return randomData;
	}

	void WriteWholeBlockToOffset(const IByteStreamPtr& stream, s64 offset, ConstByteData block, TimeDuration sleepOnBusy)
	{
		stream->Seek(offset);

		size_t total = 0;
		size_t written = 0;
		do {
			written = (size_t)stream->Write(ConstByteData(block, total));
			total += written;
			if (written == 0)
				Thread::Sleep(sleepOnBusy);
		} while (total != block.size());
	}

}


TEST(AsyncByteStreamTest, PushVODStyleWriting)
{
	static const s64 StreamDelayMs = 3;

	static const size_t AsyncPageSize = 231;

	static const size_t SubStreams = 61;
	static const size_t SubStreamSize = 95852;
	static const size_t TailStreamSize = 69234;

	static const size_t SubOffsetShift = (((SubStreamSize / AsyncPageSize) * 5) / 9) * AsyncPageSize;
	static const size_t SrcSize = SubStreams * SubStreamSize + TailStreamSize;

	ByteArray srcData(GenerateRandomArray(SrcSize));
	ByteArray dstData(srcData.size());
	ISyncableByteStreamPtr asyncStream = make_shared_ptr<AsyncByteStream>("TestStream", make_shared_ptr<DelayedMemoryByteStream<ByteArray> >(dstData, TimeDuration(StreamDelayMs)), AsyncByteStream::Config().PageSize(AsyncPageSize).SubStreamsHint(SubStreams + 1));

	Logger::Info() << "Writing...";
	for (size_t stage = 0; stage < 2; stage++)
	{
		size_t BeginOffset = 0;
		size_t EndOffset = 0;
		if (stage == 0)
		{
			BeginOffset = SubOffsetShift;
			EndOffset = SubStreamSize;
		}
		else if (stage == 1)
		{
			BeginOffset = 0;
			EndOffset = SubOffsetShift;
		}
		else
			STINGRAYKIT_THROW(NotImplementedException());

		for (size_t subOffset = BeginOffset; subOffset < EndOffset; subOffset += AsyncPageSize)
		{
			for (size_t idx = 0; idx < SubStreams; idx++)
			{
				const size_t offset = (idx * SubStreamSize) + subOffset;
				WriteWholeBlockToOffset(asyncStream, (s64)offset, ConstByteData(srcData, offset, std::min(AsyncPageSize, EndOffset - subOffset)), TimeDuration(StreamDelayMs) * (SubStreams + 1));
			}

			const size_t tailOffset = ((SubStreams * SubStreamSize) + subOffset);
			if (tailOffset < SrcSize)
				WriteWholeBlockToOffset(asyncStream, (s64)tailOffset, ConstByteData(srcData, tailOffset, std::min(AsyncPageSize, std::min(TailStreamSize, EndOffset) - subOffset)), TimeDuration(StreamDelayMs) * (SubStreams + 1));
		}
	}

	Logger::Info() << "Writing done. Syncing...";
	asyncStream->Sync();

	ASSERT_EQ(dstData.size(), srcData.size()) << "Sizes mismatch";
	ASSERT_EQ(dstData, srcData) << "Contents mismatch";
}


TEST(AsyncByteStreamTest, OverlappedWriting)
{
	static const s64 StreamDelayMs = 3;

	static const size_t AsyncPageSize = 587;
	static const size_t WritePageSize = 921;
	static const size_t GarbagePageSize = 432;
	static const size_t GarbagingRate = 6;
	static const size_t GarbagingOffset = 158;
	static const size_t SrcSize = WritePageSize * 5831;

	ByteArray srcData(GenerateRandomArray(SrcSize));
	ByteArray garbageData(GenerateRandomArray(srcData.size()));
	ByteArray dstData(srcData.size());
	ISyncableByteStreamPtr asyncStream = make_shared_ptr<AsyncByteStream>("TestStream", make_shared_ptr<DelayedMemoryByteStream<ByteArray> >(dstData, TimeDuration(StreamDelayMs)), AsyncByteStream::Config().PageSize(AsyncPageSize));

	Logger::Info() << "Writing...";
	for (size_t offset = 0; offset < SrcSize; offset += WritePageSize)
	{
		if ((offset != 0) && (((offset / WritePageSize) % GarbagingRate) == 0) && ((offset + GarbagingOffset) < SrcSize))
		{
			const size_t garbagingOffset = offset + GarbagingOffset;
			WriteWholeBlockToOffset(asyncStream, (s64)garbagingOffset, ConstByteData(garbageData, garbagingOffset, std::min(GarbagePageSize, SrcSize - garbagingOffset)), TimeDuration(StreamDelayMs));
		}

		WriteWholeBlockToOffset(asyncStream, (s64)offset, ConstByteData(srcData, offset, std::min(WritePageSize, SrcSize - offset)), TimeDuration(StreamDelayMs));
	}

	Logger::Info() << "Writing done. Syncing...";
	asyncStream->Sync();

	ASSERT_EQ(dstData.size(), srcData.size()) << "Sizes mismatch";
	ASSERT_EQ(dstData, srcData) << "Contents mismatch";
}
