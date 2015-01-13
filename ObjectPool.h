#ifndef STINGRAYKIT_OBJECTPOOL_H
#define STINGRAYKIT_OBJECTPOOL_H


#include <list>
#include <stingray/toolkit/function/bind.h>
#include <stingray/toolkit/shared_ptr.h>
#include <stingray/toolkit/thread/Thread.h>


namespace stingray
{


	class ObjectPool
	{
		STINGRAYKIT_NONCOPYABLE(ObjectPool);

		struct IObjectHolder
		{
			virtual ~IObjectHolder() { }
			virtual bool TryRemove() = 0;
		};
		STINGRAYKIT_DECLARE_PTR(IObjectHolder);

		template < typename T >
		struct ObjectHolder : public virtual IObjectHolder
		{
			shared_ptr<T>	Val;
			ObjectHolder(const shared_ptr<T>& val) : Val(val) { }
			virtual bool TryRemove() { return Val.release_if_unique(); }
		};

		typedef std::list<IObjectHolderPtr>		ObjectHolders;

	private:
		ObjectHolders	_objects;
		Mutex			_mutex;

	public:
		ObjectPool() { }

		template < typename T >
		void AddObject(const shared_ptr<T>& obj)
		{ MutexLock l(_mutex); _objects.push_back(make_shared<ObjectHolder<T> >(obj)); }

		void CollectGarbage()
		{
			MutexLock l(_mutex);
			_objects.erase(std::remove_if(_objects.begin(), _objects.end(), bind(&IObjectHolder::TryRemove, _1)), _objects.end());
		}
	};


}


#endif
