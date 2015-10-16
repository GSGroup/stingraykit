#ifndef STINGRAYKIT_PHOENIXSINGLETON_H
#define STINGRAYKIT_PHOENIXSINGLETON_H

// Copyright (c) 2011 - 2015, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/CheckedDelete.h>
#include <stingraykit/thread/atomic/AtomicFlag.h>
#include <stingraykit/thread/atomic/AtomicInt.h>
#include <stingraykit/thread/atomic/Spinlock.h>

#include <stdint.h>
#include <stdlib.h>

namespace stingray
{

#define STINGRAYKIT_PHOENIXSINGLETON(ClassName) \
		friend class ::stingray::PhoenixSingleton<ClassName>; \
		STINGRAYKIT_NONCOPYABLE(ClassName)


	template <typename T>
	class PhoenixSingleton
	{
		typedef T InstanceType;
		typedef BasicAtomicInt<intptr_t> AtomicInstance;

	public:
		static T& Instance()
		{
			InstanceType* instance = reinterpret_cast<InstanceType*>(AtomicInstance::Load(s_instance));
			if (!instance)
			{
				Spinlock l(s_lock);
				instance = reinterpret_cast<InstanceType*>(AtomicInstance::Load(s_instance));
				if (instance)
					return *instance;
				instance = new InstanceType();
				AtomicInstance::Store(s_instance, (intptr_t)instance);
				atexit(do_atexit);
			}
			return *instance;
		}

	private:
		static AtomicFlag::Type		s_lock;
		static AtomicInstance::Type	s_instance;

		static void do_atexit()
		{
			Spinlock l(s_lock);
			InstanceType* instance = reinterpret_cast<InstanceType*>(AtomicInstance::Load(s_instance));
			assert(instance);
			CheckedDelete(instance);
			AtomicInstance::Store(s_instance, (intptr_t)0);
		}
	};

	template <typename T> AtomicFlag::Type PhoenixSingleton<T>::s_lock = 0;
	template <typename T> BasicAtomicInt<intptr_t>::Type PhoenixSingleton<T>::s_instance = 0;


}


#endif

