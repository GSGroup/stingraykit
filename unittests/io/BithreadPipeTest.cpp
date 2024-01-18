// Copyright (c) 2011 - 2024, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/io/BithreadPipe.h>

#include <stingraykit/function/bind.h>
#include <stingraykit/future.h>
#include <stingraykit/thread/TimedCancellationToken.h>

#include <gtest/gtest.h>

using namespace stingray;

namespace
{

	void CheckedRead(const IPipePtr& pipe, size_t size, const ICancellationToken& token)
	{
		ByteArray data(size);
		size_t readSize = 0;

		do
		{ readSize = pipe->Read(data, token); }
		while (token && !readSize);

		ASSERT_EQ(readSize, size);
		for (size_t i = 0; i < readSize; ++i)
			ASSERT_EQ(data[i] % size, i);
	}

	void PipeRead(const IPipePtr& pipe, const ICancellationToken& token)
	{
		CheckedRead(pipe, 50, token);
		CheckedRead(pipe, 100, token);
		CheckedRead(pipe, 100, token);
		CheckedRead(pipe, 100, token);
	}

	void PipeWrite(const IPipePtr& pipe, size_t size, const ICancellationToken& token)
	{
		ByteArray data(size);
		for (size_t i = 0; i < size; ++i)
			data[i] = i;
		ASSERT_EQ(pipe->Write(data, token), size);
	}

	const size_t N = 1024;

	void DoRead(const IPipePtr& pipe, promise<size_t>& promise, const ICancellationToken& token)
	{
		ByteArray data(N / 10);
		size_t accumulatedSize = 0;
		try
		{
			while (token && accumulatedSize < N)
			{
				accumulatedSize += pipe->Read(data, token);
				Logger::Info() << "accumulatedSize: " << accumulatedSize;
			}
			promise.set_value(accumulatedSize);
		}
		catch (const std::exception& ex)
		{
			promise.set_exception(MakeExceptionPtr(ex));
		}
	}

	void DoWrite(const IPipePtr& pipe, promise<size_t>& promise, const ICancellationToken& token)
	{
		ByteArray data(N);
		size_t dataSent = 0;
		try
		{
			while (token && dataSent < N)
			{
				dataSent += pipe->Write(ConstByteData(data, dataSent, std::min(N - dataSent, N / 10 + 1)), token);
				Logger::Info() << "dataSent: " << dataSent;
			}
			promise.set_value(dataSent);
		}
		catch (const std::exception& ex)
		{
			promise.set_exception(MakeExceptionPtr(ex));
		}
	}

	void CheckCancellation(const std::string& threadName, const function<void(const IPipePtr&, promise<size_t>&, const ICancellationToken&)>& func)
	{
		promise<size_t> promise;
		future<size_t> future = promise.get_future();

		ThreadPtr worker = make_shared_ptr<Thread>(threadName, Bind(func, make_shared_ptr<BithreadPipe>(), wrap_ref(promise), _1));
		Thread::Sleep(TimeDuration(100));
		worker.reset();

		ASSERT_TRUE(future.is_ready());
		ASSERT_FALSE(future.has_value());
		ASSERT_TRUE(future.has_exception());

		bool gotOperationCancelled = false;
		try { future.get(); }
		catch (const RethrownException&) { gotOperationCancelled = true; }
		catch (const std::exception& ex) { Logger::Warning() << ex; }
		ASSERT_TRUE(gotOperationCancelled);
	}

	void CheckTimeout(const std::string& threadName, const function<void(const IPipePtr&, promise<size_t>&, const ICancellationToken&)>& func)
	{
		promise<size_t> promise;
		future<size_t> future = promise.get_future();

		Thread worker(threadName, Bind(func, make_shared_ptr<BithreadPipe>(), wrap_ref(promise), _1), TimeDuration(100));

		ASSERT_TRUE(future.wait(TimedCancellationToken(TimeDuration(200))) == future_status::ready);
		ASSERT_FALSE(future.has_value());
		ASSERT_TRUE(future.has_exception());

		bool gotTimeoutException = false;
		try { future.get(); }
		catch (const RethrownException&) { gotTimeoutException = true; }
		catch (const std::exception& ex) { Logger::Warning() << ex; }
		ASSERT_TRUE(gotTimeoutException);
	}

}

TEST(BithreadPipe, Test)
{
	const IPipePtr pipe = make_shared_ptr<BithreadPipe>();
	const ThreadPtr thread(new Thread("bithreadPipeReader", Bind(&PipeRead, pipe, _1)));
	PipeWrite(pipe, 50, DummyCancellationToken());
	PipeWrite(pipe, 100, DummyCancellationToken());
	PipeWrite(pipe, 200, DummyCancellationToken());
}

TEST(BithreadPipeConcurrent, Generic)
{
	promise<size_t> readerPromise, writerPromise;
	future<size_t> readerFuture = readerPromise.get_future(), writerFuture = writerPromise.get_future();

	IPipePtr pipe = make_shared_ptr<BithreadPipe>();

	Thread reader("reader", Bind(&DoRead, wrap_const_ref(pipe), wrap_ref(readerPromise), _1));
	Thread writer("writer", Bind(&DoWrite, wrap_const_ref(pipe), wrap_ref(writerPromise), _1));

	ASSERT_TRUE(readerFuture.wait(TimedCancellationToken(TimeDuration::FromSeconds(1))) == future_status::ready);
	ASSERT_TRUE(writerFuture.wait(TimedCancellationToken(TimeDuration::FromSeconds(1))) == future_status::ready);
	ASSERT_EQ(readerFuture.get(), writerFuture.get());
}

TEST(BithreadPipeConcurrent, ReadCancelled)
{
	CheckCancellation("reader", &DoRead);
}

TEST(BithreadPipeConcurrent, WriteCancelled)
{
	CheckCancellation("writer", &DoWrite);
}

TEST(BithreadPipeConcurrent, ReadTimedOut)
{
	CheckTimeout("reader", &DoRead);
}

TEST(BithreadPipeConcurrent, WriteTimedOut)
{
	CheckTimeout("writer", &DoWrite);
}

TEST(BithreadPipeConcurrent, WriteCancelledReadStuck)
{
	promise<size_t> readerPromise, writerPromise;
	future<size_t> readerFuture = readerPromise.get_future(), writerFuture = writerPromise.get_future();

	IPipePtr pipe = make_shared_ptr<BithreadPipe>();

	Thread writer("writer", Bind(&DoWrite, wrap_const_ref(pipe), wrap_ref(writerPromise), _1), TimeDuration(100));

	ASSERT_TRUE(writerFuture.wait(TimedCancellationToken(TimeDuration(200))) == future_status::ready);
	ASSERT_FALSE(writerFuture.has_value());
	ASSERT_TRUE(writerFuture.has_exception());

	bool gotTimeoutException = false;
	try { u8 singleByte; pipe->Read(ByteData(&singleByte, 1), TimedCancellationToken(TimeDuration(100))); }
	catch (const TimeoutException&) { gotTimeoutException = true; }
	catch (const std::exception& ex) { Logger::Warning() << ex; }
	ASSERT_TRUE(gotTimeoutException);
}
