#ifndef STINGRAY_STINGRAYKIT_PHOENIXSINGLETON_H
#define STINGRAY_STINGRAYKIT_PHOENIXSINGLETON_H

#include <stdint.h>
#include <stdlib.h>

#include <stingray/toolkit/Atomic.h>

namespace stingray
{

#define STINGRAYKIT_PHOENIXSINGLETON(ClassName) \
		friend class ::stingray::PhoenixSingleton<ClassName>; \
		STINGRAYKIT_NONCOPYABLE(ClassName)


	template <typename T>
	class PhoenixSingleton
	{
		typedef T InstanceType;

	public:
		static T& Instance()
		{
			InstanceType* instance = reinterpret_cast<InstanceType*>(Atomic::Load(s_instance));
			if (!instance)
			{
				Spinlock l(s_lock);
				instance = reinterpret_cast<InstanceType*>(Atomic::Load(s_instance));
				if (instance)
					return *instance;
				instance = new InstanceType();
				Atomic::Store(s_instance, (intptr_t)instance);
				atexit(do_atexit);
			}
			return *instance;
		}

	private:
		static atomic_int_type	s_lock;
		static intptr_t			s_instance;

		static void do_atexit()
		{
			Spinlock l(s_lock);
			InstanceType* instance = reinterpret_cast<InstanceType*>(Atomic::Load(s_instance));
			assert(instance);
			delete instance;
			Atomic::Store(s_instance, (intptr_t)0);
		}
	};

	template <typename T> atomic_int_type PhoenixSingleton<T>::s_lock = 0;
	template <typename T> intptr_t PhoenixSingleton<T>::s_instance = 0;


}


#endif

