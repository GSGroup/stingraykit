#ifndef STINGRAYKIT_SAFESINGLETON_H
#define STINGRAYKIT_SAFESINGLETON_H

// Copyright (c) 2011 - 2015, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.


#include <stingraykit/log/SystemLogger.h>
#include <stingraykit/thread/call_once.h>
#include <stingraykit/Atomic.h>
#include <stingraykit/function/bind.h>

namespace stingray
{

	template < typename T >
	class SafeSingleton
	{
	private:
		static STINGRAYKIT_DECLARE_ONCE_FLAG(s_initFlag);
		static u32		s_refCount;
		static intptr_t	s_instance;

	private:
		static bool TryAddReference()
		{
			u32 refcount = Atomic::Load(s_refCount);
			while (refcount != 0)
			{
				u32 newrefcount = Atomic::CompareAndExchange(s_refCount, refcount, refcount + 1);
				if (newrefcount == refcount)
					return true;
				refcount = newrefcount;
			}
			return false;
		}

		static void RemoveReference()
		{
			u32 refcount = Atomic::Dec(s_refCount);
			if (refcount == 0)
				delete DoGetInstancePtr();
		}

		static T* DoGetInstancePtr()
		{
			intptr_t ptr = Atomic::Load(s_instance);
			return (T*)ptr;
		}

		struct InstanceHolder
		{
			InstanceHolder()
			{
				T* instance = new T();
				Atomic::Store(s_instance, (intptr_t)instance);
				Atomic::Store(s_refCount, 1);
			}

			~InstanceHolder()
			{ RemoveReference(); }
		};

		static void DoInitInstance()
		{
			static InstanceHolder instanceHolder;
		}

		static void InitInstance()
		{
			try
			{ DoInitInstance(); }
			catch(const std::exception& ex)
			{
				StringBuilder message;
				message % "An exception in " % Demangle(typeid(T).name()) % " singleton constructor: " % diagnostic_information(ex);
				SystemLogger::Log(LogLevel::Error, message);
			}
		}

	public:
		static shared_ptr<T> Instance()
		{
			call_once(s_initFlag, &SafeSingleton::InitInstance);

			if (!TryAddReference())
				return null;
			T* instance = DoGetInstancePtr();
			if (!instance)
			{
				RemoveReference();
				return null;
			}

			return shared_ptr<T>(instance, bind(&RemoveReference, not_using(_1)));
		}
	};


	template< typename T >
	STINGRAYKIT_DEFINE_ONCE_FLAG(SafeSingleton<T>::s_initFlag);


	template < typename T >
	u32 SafeSingleton<T>::s_refCount = 0;


	template < typename T >
	intptr_t SafeSingleton<T>::s_instance = 0;

}

#endif
