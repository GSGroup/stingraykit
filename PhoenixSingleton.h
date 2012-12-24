#ifndef STINGRAY_TOOLKIT_PHOENIXSINGLETON_H
#define STINGRAY_TOOLKIT_PHOENIXSINGLETON_H

#include <stdlib.h>

namespace stingray
{


	template < typename T, typename ContextType = T >
	class PhoenixSingleton : public T
	{
		typedef PhoenixSingleton<T, ContextType> ThisT;

	public:
		static T& Instance()
		{
			static ThisT meyers;
			if (ThisT::_destroyed)
			{
				new(&meyers) ThisT;
				atexit(ThisT::do_atexit);
			}
			return meyers;
		}

	private:
		static bool _destroyed;

		PhoenixSingleton()	{ _destroyed = false; }
		~PhoenixSingleton()	{ _destroyed = true; }

		static void do_atexit()
		{
			if (_destroyed)
				return;
			static_cast<ThisT&>(Instance()).~PhoenixSingleton();
		}
	};
	template <typename T, typename C> bool PhoenixSingleton<T, C>::_destroyed = false;


}


#endif

