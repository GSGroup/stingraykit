#ifndef STINGRAY_TOOLKIT_PHOENIXSINGLETON_H
#define STINGRAY_TOOLKIT_PHOENIXSINGLETON_H

#include <stdlib.h>
#include <stingray/toolkit/Atomic.h>

namespace stingray
{


	template < typename T, typename ContextType = T >
	class PhoenixSingleton : public T
	{
		typedef PhoenixSingleton<T, ContextType> ThisT;

	public:
		static T& Instance()
		{
			ThisT* instance = Atomic::Load(ThisT::s_instance);
			if (!instance)
			{
				Spinlock l(s_lock);
				instance = Atomic::Load(ThisT::s_instance);
				if (instance)
					return *instance;
				instance = new ThisT();
				Atomic::Store(s_instance, instance);
				atexit(ThisT::do_atexit);
			}
			return *instance;
		}

	private:
		static atomic_int_type	s_lock;
		static ThisT*			s_instance;

		static void do_atexit()
		{
			Spinlock l(s_lock);
			ThisT* instance = Atomic::Load(s_instance);
			assert(instance);
			delete instance;
			Atomic::Store(s_instance, (ThisT*)0);
		}
	};

	template <typename T, typename C> atomic_int_type PhoenixSingleton<T, C>::s_lock = 0;
	template <typename T, typename C> PhoenixSingleton<T, C>* PhoenixSingleton<T, C>::s_instance = 0;


}


#endif

