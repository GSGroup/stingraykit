#ifndef STINGRAYKIT_SINGLETON_H
#define STINGRAYKIT_SINGLETON_H

// Copyright (c) 2011 - 2022, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/log/Logger.h>
#include <stingraykit/thread/call_once.h>
#include <stingraykit/unique_ptr.h>

#define STINGRAYKIT_SINGLETON(ClassName) \
		friend void ::stingray::CheckedDelete<ClassName>(ClassName* t); \
		friend class ::stingray::Singleton<ClassName>; \
		STINGRAYKIT_NONCOPYABLE(ClassName)

#define STINGRAYKIT_SINGLETON_WITH_TRIVIAL_CONSTRUCTOR(ClassName) \
		STINGRAYKIT_SINGLETON(ClassName); \
	private: \
		ClassName() { }

namespace stingray
{

	template < typename T >
	class Singleton
	{
	private:
		using InstancePtr = unique_ptr<T>;

		struct InstanceHolder
		{
			InstancePtr		Inst;
			bool			Created = false;

			~InstanceHolder() { Inst.reset(); }
		};

	private:
		static STINGRAYKIT_DECLARE_ONCE_FLAG(s_initFlag);

	public:
		static bool IsAlive()
		{ return GetInstanceHolder().Inst.is_initialized(); }

		static T& Instance()
		{
			call_once(s_initFlag, &Singleton::InitInstance);

			const InstanceHolder& holder = GetInstanceHolder();
			STINGRAYKIT_CHECK(holder.Inst, StringBuilder() % "Singleton '" % TypeInfo(typeid(T)) % "' " % (holder.Created ? "already destroyed" : "could not be created"));

			return *holder.Inst;
		}

		static const T& ConstInstance()
		{ return Instance(); }

		static void AssertInstanceCreated()
		{ call_once(s_initFlag, &Singleton::AssertInstance); }

	private:
		static void InitInstance()
		{
			InstancePtr ptr;

			try
			{ ptr.reset(new T()); }
			catch(const std::exception& ex)
			{ Logger::Error() << "An exception in " << TypeInfo(typeid(T)) << " singleton constructor:\n" << ex; }

			InstanceHolder& holder = GetInstanceHolder();

			holder.Inst = std::move(ptr);
			holder.Created = holder.Inst.is_initialized();
		}

		static void AssertInstance()
		{ STINGRAYKIT_FATAL(StringBuilder() % "Singleton '" % TypeInfo(typeid(T)) % "' has not been created"); }

		static InstanceHolder& GetInstanceHolder()
		{
			static InstanceHolder inst;
			return inst;
		}
	};

	template < typename T >
	STINGRAYKIT_DEFINE_ONCE_FLAG(Singleton<T>::s_initFlag);


	namespace Detail
	{
		template < typename T >
		TrueType	TestIsSingleton(const Singleton<T>*);
		FalseType	TestIsSingleton(...);
	}
	template < typename T > struct IsSingleton : integral_constant<bool, decltype(Detail::TestIsSingleton((T*)0))::Value> { };

}

#endif
