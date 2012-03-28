#ifndef __GS_STINGRAY_TOOLKIT_OBJECTPOOL_H__
#define __GS_STINGRAY_TOOLKIT_OBJECTPOOL_H__


#include <list>
#include <stingray/toolkit/shared_ptr.h>
#include <stingray/threads/Thread.h>


namespace stingray
{


	class ObjectPool
	{
		TOOLKIT_NONCOPYABLE(ObjectPool);

		struct IObjectHolder
		{
			virtual ~IObjectHolder() { }
			virtual bool CanBeRemoved() const = 0;
		};
		TOOLKIT_DECLARE_PTR(IObjectHolder);

		template < typename T >
		struct ObjectHolder : public virtual IObjectHolder
		{
			shared_ptr<T>	Val;
			ObjectHolder(const shared_ptr<T>& val) : Val(val) { }
			virtual bool CanBeRemoved() const { return Val.unique(); }
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
		{ MutexLock l(_mutex); _objects.erase(std::remove_if(_objects.begin(), _objects.end(), bind(&IObjectHolder::CanBeRemoved, _1)), _objects.end()); }
	};


}


#endif
