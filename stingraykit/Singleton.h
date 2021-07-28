#ifndef STINGRAYKIT_SINGLETON_H
#define STINGRAYKIT_SINGLETON_H

// Copyright (c) 2011 - 2019, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/log/Logger.h>
#include <stingraykit/thread/call_once.h>
#include <stingraykit/unique_ptr.h>

#define STINGRAYKIT_SINGLETON(ClassName) \
		friend class ::stingray::Detail::SingletonInstanceHolder<ClassName>; \
		friend class ::stingray::Singleton<ClassName>; \
		STINGRAYKIT_NONCOPYABLE(ClassName)

#define STINGRAYKIT_SINGLETON_WITH_TRIVIAL_CONSTRUCTOR(ClassName) \
		STINGRAYKIT_SINGLETON(ClassName); \
	private: \
		ClassName() { }

namespace stingray
{

	namespace Detail
	{
		template < typename T >
		class SingletonInstanceHolder : public T
		{
			STINGRAYKIT_NONCOPYABLE(SingletonInstanceHolder);

		public:
			SingletonInstanceHolder()	{ }
			T& Get()					{ return *this; }
		};
	}

	template < typename T >
	class Singleton
	{
		using InstanceHolderType = Detail::SingletonInstanceHolder<T>;
		using InstanceHolderTypePtr = unique_ptr<InstanceHolderType>;

		static void InitInstance()
		{
			InstanceHolderTypePtr ptr;
			try
			{ ptr.reset(new InstanceHolderType()); }
			catch(const std::exception& ex)
			{ Logger::Error() << "An exception in " << Demangle(typeid(T).name()) << " singleton constructor: " << ex; }
			ptr.swap(GetInstancePtr());
		}

		static void AssertInstance()
		{ STINGRAYKIT_FATAL("Singleton '" + Demangle(typeid(T).name()) + "' has not been created!"); }

		static InstanceHolderTypePtr& GetInstancePtr()
		{
			static InstanceHolderTypePtr inst;
			return inst;
		}

		static STINGRAYKIT_DECLARE_ONCE_FLAG(s_initFlag);

	public:
		static bool IsAlive()
		{ return GetInstancePtr().is_initialized(); }

		static T& Instance()
		{
			call_once(s_initFlag, &Singleton::InitInstance);
			if (!GetInstancePtr())
				STINGRAYKIT_THROW("Singleton '" + Demangle(typeid(T).name()) + "' could not be created!");
			return GetInstancePtr()->Get();
		}

		static const T& ConstInstance()
		{ return Instance(); }

		static void AssertInstanceCreated()
		{ call_once(s_initFlag, &Singleton::AssertInstance); }
	};

	template < typename T >
	STINGRAYKIT_DEFINE_ONCE_FLAG(Singleton<T>::s_initFlag);


	namespace Detail
	{
		template < typename T >
		YesType	TestIsSingleton(const Singleton<T>*);
		NoType	TestIsSingleton(...);
	}
	template < typename T > struct IsSingleton : integral_constant<bool, sizeof(Detail::TestIsSingleton((T*)0)) == sizeof(YesType)> { };

}

#endif
