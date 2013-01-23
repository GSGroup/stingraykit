#ifndef STINGRAY_TOOLKIT_PHOENIXSINGLETON_H
#define STINGRAY_TOOLKIT_PHOENIXSINGLETON_H

#include <stdlib.h>
#include <stingray/toolkit/Atomic.h>

namespace stingray
{


	template <typename T>
	class PhoenixSingleton
	{
		typedef T InstanceType;

	public:
		static T& Instance()
		{
			InstanceType* instance = (InstanceType*)Atomic::Load(s_instance);
			if (!instance)
			{
				Spinlock l(s_lock);
				instance = (InstanceType*)Atomic::Load(s_instance);
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
			InstanceType* instance = (InstanceType*)Atomic::Load(s_instance);
			assert(instance);
			delete instance;
			Atomic::Store(s_instance, (intptr_t)0);
		}
	};

	template <typename T> atomic_int_type PhoenixSingleton<T>::s_lock = 0;
	template <typename T> intptr_t PhoenixSingleton<T>::s_instance = 0;


}


#endif

