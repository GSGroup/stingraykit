#ifndef STINGRAYKIT_THREAD_POSIX_THREADLOCAL_H
#define STINGRAYKIT_THREAD_POSIX_THREADLOCAL_H


#ifdef STINGRAYKIT_HAS_THREAD_KEYWORD
#	define STINGRAYKIT_DECLARE_THREAD_LOCAL_POD(Type_, Name_) \
	struct Name_ \
	{ \
	private: \
		static __thread Type_ s_value; \
	public: \
		static Type_ Get() { return s_value; } \
		static void Set(Type_ value) { s_value = value; } \
	};

#	define STINGRAYKIT_DEFINE_THREAD_LOCAL_POD(Type_, Name_, DefaultValue_) __thread Type_ Name_::s_value = DefaultValue_
#else
#	error "No thread local storage!"
#endif


#endif
