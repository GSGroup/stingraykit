#ifndef STINGRAY_TOOLKIT_TYPEINFO_H
#define STINGRAY_TOOLKIT_TYPEINFO_H


#include <string>
#include <typeinfo>

#include <stingray/toolkit/toolkit.h>


namespace stingray
{

	class TypeInfo
	{
	private:
		const std::type_info*		_info;

	public:
		template < typename T >
		TypeInfo(const T& obj)
			: _info(&typeid(obj))
		{ }

		std::string GetName() const		{ return Demangle(GetRawName()); }
		const char* GetRawName() const	{ return _info->name(); }

		std::string ToString() const	{ return GetName(); }
	};

}

#endif
