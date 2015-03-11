#ifndef STINGRAYKIT_THREAD_POSIX_THREADLOCAL_H
#define STINGRAYKIT_THREAD_POSIX_THREADLOCAL_H

// Copyright (c) 2011 - 2015, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/SafeSingleton.h>
#include <stingraykit/SystemException.h>
#include <stingraykit/unique_ptr.h>

#include <pthread.h>

#if defined(STINGRAYKIT_HAS_THREAD_KEYWORD) && defined(PLATFORM_POSIX)

#ifdef __mips__
#define DETAIL_STINGRAYKIT_TLS_GET_ATTR __attribute__((nomips16, noinline))
#else
#define DETAIL_STINGRAYKIT_TLS_GET_ATTR
#endif

#	define STINGRAYKIT_DECLARE_THREAD_LOCAL(Type_, Name_) \
	struct Name_ \
	{ \
	private: \
		struct KeyHolder \
		{ \
		private: \
			pthread_key_t	_key; \
		public: \
			KeyHolder() \
			{ \
				int res = pthread_key_create(&_key, &Name_::Dtor); \
				if (res != 0) \
					STINGRAYKIT_FATAL(StringBuilder() % "pthread_key_create failed: " % SystemException::GetErrorMessage(res)); \
			} \
			~KeyHolder() \
			{ \
				int res = pthread_key_delete(_key); \
				if (res != 0) \
					STINGRAYKIT_FATAL(StringBuilder() % "pthread_key_delete failed: " % SystemException::GetErrorMessage(res)); \
			} \
			pthread_key_t GetKey() const \
			{ return _key; } \
		}; \
		STINGRAYKIT_DECLARE_PTR(KeyHolder); \
		struct ValueHolder \
		{ \
			KeyHolderPtr	Key; \
			Type_			Value; \
			ValueHolder(const KeyHolderPtr& key) : Key(key), Value() \
			{ } \
		}; \
		static __thread Type_* s_value; \
	public: \
		DETAIL_STINGRAYKIT_TLS_GET_ATTR static Type_& Get() \
		{ \
			if (!s_value)\
			{ \
				KeyHolderPtr key = SafeSingleton<KeyHolder>::Instance(); \
				if (!key) \
					key = make_shared<KeyHolder>(); \
				unique_ptr<ValueHolder> valPtr(new ValueHolder(key)); \
				int res = pthread_setspecific(key->GetKey(), valPtr.get()); \
				STINGRAYKIT_CHECK(res == 0, SystemException("pthread_setspecific", res)); \
				s_value = &(valPtr->Value); \
				valPtr.release(); \
			} \
			return *s_value; \
		} \
	private: \
		static void Dtor(void* val) \
		{ ValueHolder* holder = static_cast<ValueHolder*>(val); delete holder; } \
	};

#	define STINGRAYKIT_DEFINE_THREAD_LOCAL(Type_, Name_) __thread Type_* Name_::s_value = NULL
#else
#	error "No thread local storage!"
#endif

#endif
