#include <stingray/toolkit/Factory.h>
#include <stingray/log/Logger.h>

/*! \cond GS_INTERNAL */

namespace stingray { namespace Detail 
{
	void Factory::Dump()
	{
		Logger::Stream(LogLevel::Info) << "Registered " << _registrars.size() << " classes:";
		for(RegistrarMap::const_iterator i = _registrars.begin(); i != _registrars.end(); ++i)
		{
			Logger::Stream(LogLevel::Info) << i->first;
		}
	}

}}

/*! \endcond */
