#ifndef STINGRAY_TOOLKIT_COMPOSITEDATACONSUMER_H
#define STINGRAY_TOOLKIT_COMPOSITEDATACONSUMER_H


#include <stingray/toolkit/IDataSource.h>

#include <list>


namespace stingray
{

	class CompositeDataConsumer : public virtual IDataConsumer
	{
		typedef std::list<IDataConsumerWeakPtr> ConsumersContainer;

	private:
		Mutex				_consumersGuard;
		ConsumersContainer	_consumers;

	public:
		void AddConsumer(const IDataConsumerPtr& consumer)
		{
			MutexLock l(_consumersGuard);
			_consumers.remove_if(bind(&IDataConsumerWeakPtr::expired, _1));
			_consumers.push_back(consumer);
		}

		virtual size_t Process(ConstByteData data, const ICancellationToken& token)
		{
			ForEachConsumer(bind(&CompositeDataConsumer::ProcessImpl, _1, data, ref(token)));
			return data.size();
		}

		virtual void EndOfData()
		{ ForEachConsumer(bind(&CompositeDataConsumer::EndOfDataImpl, _1)); }

	private:
		void ForEachConsumer(const function<void (const IDataConsumerPtr&)>& func)
		{
			MutexLock l(_consumersGuard);
			std::for_each(_consumers.begin(), _consumers.end(), bind(&CompositeDataConsumer::Invoke, _1, func));
		}

		static void Invoke(const IDataConsumerWeakPtr& consumerWeak, const function<void (const IDataConsumerPtr&)>& func)
		{
			const IDataConsumerPtr consumer = consumerWeak.lock();
			if (consumer)
				func(consumer);
		}

		static void ProcessImpl(const IDataConsumerPtr& consumer, ConstByteData data, const ICancellationToken& token)
		{ TOOLKIT_CHECK(consumer->Process(data, token) == data.size(), NotImplementedException()); }

		static void EndOfDataImpl(const IDataConsumerPtr& consumer)
		{ consumer->EndOfData(); }
	};
	TOOLKIT_DECLARE_PTR(CompositeDataConsumer);

}


#endif
