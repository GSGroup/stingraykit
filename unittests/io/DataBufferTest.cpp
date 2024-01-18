// Copyright (c) 2011 - 2024, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/io/DataBuffer.h>

#include <stingraykit/collection/Range.h>
#include <stingraykit/function/functional.h>
#include <stingraykit/thread/TimedCancellationToken.h>

#include <gtest/gtest.h>

using namespace stingray;

namespace
{

	class DataCounterConsumer : public virtual IDataConsumer
	{
	private:
		size_t&		_processed;

	public:
		DataCounterConsumer(size_t& processed) : _processed(processed) { }

		size_t Process(ConstByteData data, const ICancellationToken& token) override { _processed += data.size(); return data.size(); }
		void EndOfData(const ICancellationToken& token) override { }
	};

	void WaitForData(bool& finished, size_t threshold, IDataBuffer& buffer, const ICancellationToken& token)
	{
		buffer.WaitForData(threshold, token);
		finished = true;
	}

	void PushData(IDataBuffer& buffer, u8 num, atomic<bool>& finished, const ICancellationToken& token)
	{
		for (size_t i = 0; i < 16 && token; ++i)
		{
			ByteArray data((num % 4 + 1) * 32);
			std::fill(data.begin(), data.end(), i);

			const size_t consumed = ConsumeAll(buffer, data, token);
		}

		finished = true;
	}

	class DataChecker : public virtual IDataConsumer
	{
	public:
		size_t Process(ConstByteData data, const ICancellationToken& token) override
		{
			for (size_t offset = 0; token && offset < data.size(); ++offset)
				STINGRAYKIT_CHECK(data[offset] == data[offset / 16 * 16], StringBuilder() % "Expected " % data[offset / 16 * 16] % ", got " % data[offset]);

			return data.size();
		}

		void EndOfData(const ICancellationToken& token) override { }
	};

}

TEST(DataBufferTest, Consistency)
{
	DataBuffer buffer(false, 128);
	ASSERT_EQ(buffer.GetStorageSize(), 128u);
	ASSERT_EQ(buffer.GetFreeSize(), 128u);
	ASSERT_EQ(buffer.GetDataSize(), 0u);
	ASSERT_FALSE(buffer.HasEndOfDataOrException());

	ByteArray data(32);
	ASSERT_EQ(buffer.Process(data, TimedCancellationToken(TimeDuration())), 32u);
	ASSERT_EQ(buffer.GetFreeSize(), 96u);
	ASSERT_EQ(buffer.GetDataSize(), 32u);

	size_t processed = 0;
	DataCounterConsumer consumer(processed);
	buffer.Read(consumer, TimedCancellationToken(TimeDuration()));
	ASSERT_EQ(processed, 32u);
	ASSERT_EQ(buffer.GetFreeSize(), 128u);
	ASSERT_EQ(buffer.GetDataSize(), 0u);
}

TEST(DataBufferTest, EndOfData)
{
	DataBuffer buffer(false, 128);
	buffer.EndOfData(TimedCancellationToken(TimeDuration()));
	ASSERT_TRUE(buffer.HasEndOfDataOrException());
	bool processCalled = false;
	bool eodCalled = false;
	ASSERT_NO_THROW(buffer.ReadToFunction(Bind(make_assigner(processCalled), true), Bind(make_assigner(eodCalled), true), TimedCancellationToken(TimeDuration())));
	ASSERT_FALSE(processCalled);
	ASSERT_TRUE(eodCalled);

	ASSERT_ANY_THROW(buffer.SetException(std::exception(), TimedCancellationToken(TimeDuration())));
}

TEST(DataBufferTest, Exception)
{
	DataBuffer buffer(false, 128);
	buffer.SetException(std::exception(), TimedCancellationToken(TimeDuration()));
	ASSERT_TRUE(buffer.HasEndOfDataOrException());
	bool processCalled = false;
	bool eodCalled = false;
	ASSERT_ANY_THROW(buffer.ReadToFunction(Bind(make_assigner(processCalled), true), Bind(make_assigner(eodCalled), true), TimedCancellationToken(TimeDuration())));
	ASSERT_FALSE(processCalled);
	ASSERT_FALSE(eodCalled);

	ASSERT_ANY_THROW(buffer.EndOfData(TimedCancellationToken(TimeDuration())));
}

TEST(DataBufferTest, Overflow)
{
	ByteArray data(64);

	{
		bool overflow = false;
		DataBuffer buffer(true, 128);
		const Token overflowConnection = buffer.OnOverflow().connect(Bind(make_assigner(overflow), true));
		ASSERT_EQ(buffer.Process(data, TimedCancellationToken(TimeDuration())), 64u);
		ASSERT_FALSE(overflow);
		ASSERT_EQ(buffer.Process(data, TimedCancellationToken(TimeDuration())), 64u);
		ASSERT_FALSE(overflow);
		ASSERT_EQ(buffer.Process(data, TimedCancellationToken(TimeDuration())), 64u);
		ASSERT_TRUE(overflow);
	}

	{
		bool overflow = false;
		DataBuffer buffer(false, 128);
		const Token overflowConnection = buffer.OnOverflow().connect(Bind(make_assigner(overflow), true));
		ASSERT_EQ(buffer.Process(data, TimedCancellationToken(TimeDuration())), 64u);
		ASSERT_FALSE(overflow);
		ASSERT_EQ(buffer.Process(data, TimedCancellationToken(TimeDuration())), 64u);
		ASSERT_FALSE(overflow);
		ASSERT_EQ(buffer.Process(data, TimedCancellationToken(TimeDuration())), 0u);
		ASSERT_FALSE(overflow);
	}
}

TEST(DataBufferTest, InputSize)
{
	DataBuffer buffer(false, 128, DataBuffer::Parameters().SetInputPacketSize(16));
	ByteArray data(32);
	ASSERT_EQ(buffer.Process(ByteData(data, 0, 8), TimedCancellationToken(TimeDuration())), 8u);
	ASSERT_EQ(buffer.GetDataSize(), 0u);
	ASSERT_EQ(buffer.Process(ByteData(data, 0, 16), TimedCancellationToken(TimeDuration())), 16u);
	ASSERT_EQ(buffer.GetDataSize(), 16u);
	ASSERT_EQ(buffer.Process(ByteData(data, 0, 32), TimedCancellationToken(TimeDuration())), 32u);
	ASSERT_EQ(buffer.GetDataSize(), 48u);
}

TEST(DataBufferTest, OutputSize)
{
	size_t processed = 0;
	DataBuffer buffer(false, 128, DataBuffer::Parameters().SetOutputPacketSize(16));
	ByteArray data(32);
	DataCounterConsumer consumer(processed);

	buffer.Read(consumer, TimedCancellationToken(TimeDuration()));
	ASSERT_EQ(processed, 0u);

	ASSERT_EQ(buffer.Process(ByteData(data, 0, 8), TimedCancellationToken(TimeDuration())), 8u);
	buffer.Read(consumer, TimedCancellationToken(TimeDuration()));
	ASSERT_EQ(processed, 0u);

	ASSERT_EQ(buffer.Process(ByteData(data, 0, 8), TimedCancellationToken(TimeDuration())), 8u);
	buffer.Read(consumer, TimedCancellationToken(TimeDuration()));
	ASSERT_EQ(processed, 16u);

	ASSERT_EQ(buffer.Process(ByteData(data, 0, 32), TimedCancellationToken(TimeDuration())), 32u);
	buffer.Read(consumer, TimedCancellationToken(TimeDuration()));
	ASSERT_EQ(processed, 48u);

	ASSERT_EQ(buffer.Process(ByteData(data, 0, 24), TimedCancellationToken(TimeDuration())), 24u);
	buffer.Read(consumer, TimedCancellationToken(TimeDuration()));
	ASSERT_EQ(processed, 64u);

	ASSERT_EQ(buffer.Process(ByteData(data, 0, 16), TimedCancellationToken(TimeDuration())), 16u);
	buffer.Read(consumer, TimedCancellationToken(TimeDuration()));
	ASSERT_EQ(processed, 80u);

	ASSERT_EQ(buffer.GetDataSize(), 8u);
}

TEST(DataBufferTest, Clear)
{
	DataBuffer buffer(false, 128);
	ByteArray data(32);

	ASSERT_EQ(buffer.Process(data, TimedCancellationToken(TimeDuration())), 32u);
	buffer.Clear();
	ASSERT_EQ(buffer.GetDataSize(), 0u);

	buffer.EndOfData(TimedCancellationToken(TimeDuration()));
	buffer.Clear();
	ASSERT_FALSE(buffer.HasEndOfDataOrException());

	buffer.SetException(std::exception(), TimedCancellationToken(TimeDuration()));
	buffer.Clear();
	ASSERT_FALSE(buffer.HasEndOfDataOrException());
}

TEST(DataBufferTest, WaitForData)
{
	ByteArray data(64u);

	{
		DataBuffer buffer(false, 128);
		bool finished = false;
		Thread waiter("waiter", Bind(&WaitForData, wrap_ref(finished), 32u, wrap_ref(buffer), _1));
		Thread::Sleep(TimeDuration::FromMilliseconds(10));
		ASSERT_FALSE(finished);
		buffer.Process(ByteData(data, 0, 16u), TimedCancellationToken(TimeDuration()));
		Thread::Sleep(TimeDuration::FromMilliseconds(10));
		ASSERT_FALSE(finished);
		buffer.Process(ByteData(data, 0, 16u), TimedCancellationToken(TimeDuration()));
		Thread::Sleep(TimeDuration::FromMilliseconds(10));
		ASSERT_TRUE(finished);
	}

	{
		DataBuffer buffer(false, 128);
		bool finished = false;
		Thread waiter("waiter", Bind(&WaitForData, wrap_ref(finished), 32u, wrap_ref(buffer), _1));
		Thread::Sleep(TimeDuration::FromMilliseconds(10));
		ASSERT_FALSE(finished);
		buffer.EndOfData(TimedCancellationToken(TimeDuration()));
		Thread::Sleep(TimeDuration::FromMilliseconds(10));
		ASSERT_TRUE(finished);
	}

	{
		DataBuffer buffer(false, 128);
		bool finished = false;
		Thread waiter("waiter", Bind(&WaitForData, wrap_ref(finished), 32u, wrap_ref(buffer), _1));
		Thread::Sleep(TimeDuration::FromMilliseconds(10));
		ASSERT_FALSE(finished);
		buffer.SetException(std::exception(), TimedCancellationToken(TimeDuration()));
		Thread::Sleep(TimeDuration::FromMilliseconds(10));
		ASSERT_TRUE(finished);
	}
}

TEST(DataBufferTest, ReadWrite)
{
	DataBuffer buffer(false, 128, DataBuffer::Parameters(32));
	atomic<bool> finished;
	Thread writer("writer", Bind(&PushData, wrap_ref(buffer), 3, wrap_ref(finished), _1));

	DataChecker checker;
	TimedCancellationToken token(TimeDuration::Second());
	while (!finished)
		ASSERT_NO_THROW(buffer.Read(checker, token));
}

TEST(DataBufferTest, MultipleReadWrite)
{
	DataBuffer buffer(false, 128, DataBuffer::Parameters(32));
	std::vector<ThreadPtr> writers;
	std::vector<atomic<bool>> finished(16);
	for (size_t i = 0; i < 16; ++i)
		writers.emplace_back(make_shared_ptr<Thread>(StringBuilder() % "writer" % i, Bind(&PushData, wrap_ref(buffer), i, wrap_ref(finished[i]), _1)));

	DataChecker checker;
	TimedCancellationToken token(TimeDuration::Second());
	while (std::find(finished.begin(), finished.end(), false) != finished.end())
		ASSERT_NO_THROW(buffer.Read(checker, token));
}

TEST(DataBufferTest, Deadlock)
{
	DataBuffer buffer(false, 128, DataBuffer::Parameters(32));
	atomic<bool> finished;
	Thread writer1("writer1", Bind(&PushData, wrap_ref(buffer), 2, wrap_ref(finished), _1));
	Thread::Sleep(TimeDuration::FromMilliseconds(10));
	Thread writer2("writer2", Bind(&PushData, wrap_ref(buffer), 3, wrap_ref(finished), _1));
}
