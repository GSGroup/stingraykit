#ifndef STINGRAYKIT_IO_COMPOSITEDATACONSUMER_H
#define STINGRAYKIT_IO_COMPOSITEDATACONSUMER_H

// Copyright (c) 2011 - 2015, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.


#include <stingraykit/log/Logger.h>
#include <stingraykit/io/IDataSource.h>
#include <stingraykit/function/functional.h>

#include <list>


namespace stingray
{

	class CompositeDataConsumer : public virtual IDataConsumer
	{
		STINGRAYKIT_DECLARE_SIMPLE_EXCEPTION(ProcessingCancelledException, "Processing have been cancelled!");

		typedef std::list<IDataConsumerWeakPtr> ConsumersContainer;

	private:
		static NamedLogger	s_logger;

		Mutex				_guard;

		ConsumersContainer	_consumers;

	public:
		void AddConsumer(const IDataConsumerPtr& consumer)
		{
			MutexLock l(_guard);
			_consumers.remove_if(bind(&IDataConsumerWeakPtr::expired, _1));
			_consumers.push_back(consumer);
		}

		bool HasConsumers()
		{
			MutexLock l(_guard);
			return std::find_if(_consumers.begin(), _consumers.end(), not_(bind(&IDataConsumerWeakPtr::expired, _1))) != _consumers.end();
		}

		virtual size_t Process(ConstByteData data, const ICancellationToken& token)
		{
			try { ForEachConsumer(bind(&CompositeDataConsumer::ProcessImpl, _1, data, const_ref(token))); }
			catch (const ProcessingCancelledException& ex) { }
			return data.size();
		}

		virtual void EndOfData(const ICancellationToken& token)
		{ ForEachConsumer(bind(&CompositeDataConsumer::EndOfDataImpl, _1, const_ref(token))); }

	private:
		void ForEachConsumer(const function<void (const IDataConsumerPtr&)>& func)
		{
			MutexLock l(_guard);
			std::for_each(_consumers.begin(), _consumers.end(), bind(&CompositeDataConsumer::Invoke, _1, func));
		}

		static void Invoke(const IDataConsumerWeakPtr& consumerWeak, const function<void (const IDataConsumerPtr&)>& func)
		{
			const IDataConsumerPtr consumer = consumerWeak.lock();
			if (consumer)
				func(consumer);
		}

		static void ProcessImpl(const IDataConsumerPtr& consumer, ConstByteData data, const ICancellationToken& token)
		{
			for (size_t offset = 0; offset < data.size(); )
			{
				STINGRAYKIT_CHECK(token, ProcessingCancelledException());
				const size_t processed = consumer->Process(ConstByteData(data, offset), token);
				if (processed == 0)
				{
					s_logger.Warning() << "Process() returned zero!";
					Thread::Sleep(TimeDuration(100));
				}
				offset += processed;
			}
		}

		static void EndOfDataImpl(const IDataConsumerPtr& consumer, const ICancellationToken& token)
		{ consumer->EndOfData(token); }
	};
	STINGRAYKIT_DECLARE_PTR(CompositeDataConsumer);

}


#endif
