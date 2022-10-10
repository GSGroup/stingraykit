#ifndef STINGRAYKIT_SAFESINGLETON_H
#define STINGRAYKIT_SAFESINGLETON_H

// Copyright (c) 2011 - 2022, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/function/bind.h>
#include <stingraykit/log/SystemLogger.h>
#include <stingraykit/string/ToString.h>
#include <stingraykit/thread/atomic/AtomicInt.h>
#include <stingraykit/thread/call_once.h>

namespace stingray
{

	template < typename T >
	class SafeSingleton
	{
		typedef BasicAtomicInt<intptr_t> AtomicInstance;

	private:
		static STINGRAYKIT_DECLARE_ONCE_FLAG(s_initFlag);
		static AtomicU32::Type      s_refCount;
		static AtomicInstance::Type s_instance;

	private:
		static bool TryAddReference()
		{
			u32 refcount = AtomicU32::Load(s_refCount);
			while (refcount != 0)
			{
				u32 newrefcount = AtomicU32::CompareAndExchange(s_refCount, refcount, refcount + 1);
				if (newrefcount == refcount)
					return true;
				refcount = newrefcount;
			}
			return false;
		}

		static void RemoveReference()
		{
			u32 refcount = AtomicU32::Dec(s_refCount);
			if (refcount == 0)
				CheckedDelete(DoGetInstancePtr());
		}

		static T* DoGetInstancePtr()
		{
			intptr_t ptr = AtomicInstance::Load(s_instance);
			return (T*)ptr;
		}

		struct InstanceHolder
		{
			InstanceHolder()
			{
				T* instance = new T();
				AtomicInstance::Store(s_instance, (intptr_t)instance);
				AtomicU32::Store(s_refCount, 1);
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
				message % "An exception in " % Demangle(typeid(T).name()) % " singleton constructor: " % ex;
				SystemLogger::Log(LoggerMessage(LogLevel::Error, message, false));
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

			return shared_ptr<T>(instance, Bind(&RemoveReference));
		}
	};


	template< typename T >
	STINGRAYKIT_DEFINE_ONCE_FLAG(SafeSingleton<T>::s_initFlag);


	template < typename T >
	AtomicU32::Type SafeSingleton<T>::s_refCount = 0;


	template < typename T >
	BasicAtomicInt<intptr_t>::Type SafeSingleton<T>::s_instance = 0;

}

#endif
