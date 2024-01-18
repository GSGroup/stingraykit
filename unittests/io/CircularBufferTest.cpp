// Copyright (c) 2011 - 2024, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/io/BithreadCircularBuffer.h>
#include <stingraykit/io/MemoryCircularBuffer.h>
#include <stingraykit/shared_ptr.h>
#include <stingraykit/string/Hex.h>
#include <stingraykit/thread/ConditionVariable.h>

#include <limits>

#include <gtest/gtest.h>

using namespace stingray;

namespace
{

	void BufferOverflow(MemoryCircularBuffer<false> &buffer, const ConstByteData& data)
	{
		const size_t SizeBefore = buffer.GetSize();
		try
		{ buffer.Push(data); }
		catch (const BufferIsFullException&)
		{
			ASSERT_EQ(buffer.GetSize(), SizeBefore);
			return;
		}
		FAIL() << "buffer overflow";
	}


	static u32 GeneratePseudoRandomSequence(u32 prevVal)
	{ return 1664525 * prevVal + 1013904223; }


	static void ProducerFunc(const Mutex& m, ConditionVariable& emptyCv, ConditionVariable& fullCv, const BithreadCircularBufferPtr& buf, size_t count)
	{
		u32 sequence = 0;
		for (size_t i = 0; i < count; )
		{
			MutexLock l(m);
			const size_t PushLimit = 1009; // Me likes simpul numbers!
			BithreadCircularBuffer::Writer w = buf->Write();
			if (w.size() == 0)
			{
				fullCv.Wait(m);
				continue;
			}
			std::vector<u8> data(std::min(PushLimit, std::min(w.size(), count - i)));
			for (std::vector<u8>::iterator it = data.begin(); it != data.end(); ++i, ++it)
			{
				*it = (u8)(sequence & 0xFF);
				sequence = GeneratePseudoRandomSequence(sequence);
			}
			std::copy(data.begin(), data.end(), w.begin());
			//Logger::Info() << "Pushing " << data.size() << " bytes: " << ShortHexDump(data, data.size(), data.size());
			w.Push(data.size());
			emptyCv.Broadcast();
		}
	}


	static void ConsumerFunc(const Mutex& m, ConditionVariable& emptyCv, ConditionVariable& fullCv, const BithreadCircularBufferPtr& buf, size_t count)
	{
		u32 sequence = 0;
		size_t i = 0;
		while (i < count)
		{
			MutexLock l(m);
			const size_t PopLimit = 1013; // Me likes simpul numbers!
			BithreadCircularBuffer::Reader r = buf->Read();
			if (r.size() == 0)
			{
				emptyCv.Wait(m);
				continue;
			}
			ConstByteData data(r.GetData(), 0, std::min(PopLimit, r.size()));
			for (ConstByteData::iterator it = data.begin(); it != data.end(); ++i, ++it)
			{
				ASSERT_EQ(*it, sequence & 0xFF);
				sequence = GeneratePseudoRandomSequence(sequence);
			}
			//Logger::Info() << "Popping " << data.size() << " bytes: " << ShortHexDump(data, data.size(), data.size());
			r.Pop(data.size());
			fullCv.Broadcast();
		}
	}

}


TEST(CircularBufferTest, NonDisposing)
{
	MemoryCircularBuffer<false> buffer(10);
	u8 data[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11};
	ConstByteData byte_data(data, sizeof(data) / sizeof(data[0]));
	BufferOverflow(buffer, ConstByteData(byte_data, 0, 12));
	BufferOverflow(buffer, ConstByteData(byte_data, 0, 11));
	buffer.Push(ConstByteData(byte_data, 0, 3));
	ASSERT_EQ(buffer.GetSize(), 3u);
	buffer.Push(ConstByteData(byte_data, 3, 3));
	ASSERT_EQ(buffer.GetSize(), 6u);
	{
		CircularDataReserverPtr reserver = buffer.Pop();
		ConstByteData d = reserver->GetData();
		ASSERT_TRUE(std::equal(d.begin(), d.end(), data));
	}
	buffer.Push(ConstByteData(byte_data, 5, 3));
	ASSERT_EQ(buffer.GetSize(), 3u);
	{
		CircularDataReserverPtr reserver = buffer.Pop();
		ConstByteData d = reserver->GetData();
		u8 data[] = {5, 6, 7};
		ASSERT_TRUE(std::equal(d.begin(), d.end(), data));
	}
	BufferOverflow(buffer, ConstByteData(byte_data, 0, 11));
	ASSERT_EQ(buffer.GetSize(), 0u);
	{
		CircularDataReserverPtr reserver = buffer.Pop();
		ConstByteData d = reserver->GetData();
		u8 data[] = {};
		ASSERT_TRUE(std::equal(d.begin(), d.end(), data));
	}

	BufferOverflow(buffer, ConstByteData(byte_data, 0, 12));
	BufferOverflow(buffer, ConstByteData(byte_data, 0, 11));
	buffer.Push(ConstByteData(byte_data, 0, 9));
	ASSERT_EQ(buffer.GetSize(), 9u);
	{
		CircularDataReserverPtr reserver = buffer.Pop();
		ConstByteData d = reserver->GetData();
		u8 data[] = {0};
		ASSERT_TRUE(std::equal(d.begin(), d.end(), data));
	}

	BufferOverflow(buffer, ConstByteData(byte_data, 0, 3));
	BufferOverflow(buffer, ConstByteData(byte_data, 0, 11));
	ASSERT_EQ(buffer.GetSize(), 8u);
	{
		CircularDataReserverPtr reserver = buffer.Pop(4);
		ConstByteData d = reserver->GetData();
		u8 data[] = {1, 2, 3, 4};
		ASSERT_TRUE(std::equal(d.begin(), d.end(), data));
	}

	BufferOverflow(buffer, ConstByteData(byte_data, 0, 7));
	BufferOverflow(buffer, ConstByteData(byte_data, 0, 12));
	ASSERT_EQ(buffer.GetSize(), 4u);
	{
		CircularDataReserverPtr reserver = buffer.Pop(4);
		ConstByteData d = reserver->GetData();
		u8 data[] = {5, 6, 7, 8};
		ASSERT_TRUE(std::equal(d.begin(), d.end(), data));
	}

	BufferOverflow(buffer, ConstByteData(byte_data, 0, 12));
	BufferOverflow(buffer, ConstByteData(byte_data, 0, 11));
	ASSERT_EQ(buffer.GetSize(), 0u);

	buffer.Push(ConstByteData(byte_data, 0, 9));
	ASSERT_EQ(buffer.GetSize(), 9u);
	{
		shared_ptr<std::vector<u8> > d = buffer.GetAllData();
		u8 data[] = {0, 1, 2, 3, 4, 5, 6, 7, 8};
		ASSERT_TRUE(std::equal(d->begin(), d->end(), data));
		ASSERT_EQ(d->size(), buffer.GetSize());
		buffer.Pop();
		buffer.Pop();
		ASSERT_EQ(buffer.GetSize(), 0u);
	}
}


TEST(CircularBufferTest, BufferDisposing)
{
	MemoryCircularBuffer<true> buffer(10);
	u8 data[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11};
	ConstByteData byte_data(data, sizeof(data) / sizeof(data[0]));
	buffer.Push(ConstByteData(byte_data, 0, 3));
	ASSERT_EQ(buffer.GetSize(), 3u);
	buffer.Push(ConstByteData(byte_data, 3, 3));
	ASSERT_EQ(buffer.GetSize(), 6u);
	{
		CircularDataReserverPtr reserver = buffer.Pop();
		ConstByteData d = reserver->GetData();
		ASSERT_TRUE(std::equal(d.begin(), d.end(), data));
	}
	buffer.Push(ConstByteData(byte_data, 6, 6));
	ASSERT_EQ(buffer.GetSize(), 6u);
	{
		shared_ptr<std::vector<u8> > d = buffer.GetAllData();
		u8 data[] = {6, 7, 8, 9, 10, 11};
		ASSERT_TRUE(std::equal(d->begin(), d->end(), data));
	}
	buffer.Pop(2);
	ASSERT_EQ(buffer.GetSize(), 4u);
	{
		shared_ptr<std::vector<u8> > d = buffer.GetAllData();
		u8 data[] = {8, 9, 10, 11};
		ASSERT_TRUE(std::equal(d->begin(), d->end(), data));
	}
	buffer.Pop(2);
	ASSERT_EQ(buffer.GetSize(), 2u);
	{
		shared_ptr<std::vector<u8> > d = buffer.GetAllData();
		u8 data[] = {10, 11};
		ASSERT_TRUE(std::equal(d->begin(), d->end(), data));
	}
	buffer.Push(ConstByteData(byte_data, 0, 12));
	ASSERT_EQ(buffer.GetSize(), 10u);
	{
		shared_ptr<std::vector<u8> > d = buffer.GetAllData();
		u8 data[] = {2, 3, 4, 5, 6, 7, 8, 9, 10, 11};
		ASSERT_TRUE(std::equal(d->begin(), d->end(), data));
	}
	buffer.Pop(3);
	ASSERT_EQ(buffer.GetSize(), 7u);
	{
		shared_ptr<std::vector<u8> > d = buffer.GetAllData();
		u8 data[] = {5, 6, 7, 8, 9, 10, 11};
		ASSERT_TRUE(std::equal(d->begin(), d->end(), data));
	}
}


TEST(CircularBufferTest, BithreadCircularBuffer)
{
	Mutex m;
	ConditionVariable emptyCv;
	ConditionVariable fullCv;
	BithreadCircularBufferPtr buf = make_shared_ptr<BithreadCircularBuffer>(0x1000);
	size_t count = 0x10000;
	ThreadPtr producer(new Thread("circularBufferProducer", Bind(&ProducerFunc, wrap_const_ref(m), wrap_ref(emptyCv), wrap_ref(fullCv), buf, count)));
	ThreadPtr consumer(new Thread("circularBufferConsumer", Bind(&ConsumerFunc, wrap_const_ref(m), wrap_ref(emptyCv), wrap_ref(fullCv), buf, count)));
	producer.reset();
	consumer.reset();
}
