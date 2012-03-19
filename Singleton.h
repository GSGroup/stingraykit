#ifndef __GS_DVRLIB_TOOLKIT_SINGLETON_H__
#define __GS_DVRLIB_TOOLKIT_SINGLETON_H__


#include <dvrlib/toolkit/toolkit.h>


#define TOOLKIT_SINGLETON(ClassName) \
		friend class dvrlib::Singleton<ClassName>; \
		TOOLKIT_NONCOPYABLE(ClassName)

#define TOOLKIT_SINGLETON_WITH_TRIVIAL_CONSTRUCTOR(ClassName) \
		TOOLKIT_SINGLETON(ClassName); \
	private: \
		ClassName() { }
		

namespace dvrlib
{

	/** @brief Simple Singleton implementation, NOT THREAD-SAFE!!! */
	template < typename T >
	class Singleton
	{
	public:
		static T& Instance() 
		{
			static T inst;
			return inst;
		}

		static const T& ConstInstance()
		{ return const_cast<const T&>(Instance()); }
	};
	
}


#endif
