#ifndef STINGRAY_TOOLKIT_COMPOSITEDATACONSUMER_H
#define STINGRAY_TOOLKIT_COMPOSITEDATACONSUMER_H


#include <stingray/toolkit/IDataSource.h>

#include <list>


namespace stingray
{

	class CompositeDataConsumer : public virtual IDataConsumer
	{
		TOOLKIT_DECLARE_SIMPLE_EXCEPTION(ProcessingCancelledException, "Processing have been cancelled!");

		typedef std::list<IDataConsumerWeakPtr> ConsumersContainer;

	private:
		Mutex				_guard;

		ConsumersContainer	_consumers;

	public:
		void AddConsumer(const IDataConsumerPtr& consumer)
		{
			MutexLock l(_guard);
			_consumers.remove_if(bind(&IDataConsumerWeakPtr::expired, _1));
			_consumers.push_back(consumer);
		}

		virtual size_t Process(ConstByteData data, const ICancellationToken& token)
		{
			try { ForEachConsumer(bind(&CompositeDataConsumer::ProcessImpl, _1, data, ref(token))); }
			catch (const ProcessingCancelledException& ex) { }
			return data.size();
		}

		virtual void EndOfData()
		{ ForEachConsumer(bind(&CompositeDataConsumer::EndOfDataImpl, _1)); }

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
				TOOLKIT_CHECK(token, ProcessingCancelledException());
				const size_t processed = consumer->Process(ConstByteData(data, offset), token);
				offset += processed;
			}
		}

		static void EndOfDataImpl(const IDataConsumerPtr& consumer)
		{ consumer->EndOfData(); }
	};
	TOOLKIT_DECLARE_PTR(CompositeDataConsumer);

}


#endif
