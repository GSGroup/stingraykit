#ifndef STINGRAY_TOOLKIT_TYPEINFO_H
#define STINGRAY_TOOLKIT_TYPEINFO_H


#include <string>
#include <typeinfo>

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

		TypeInfo(const std::type_info& stdTypeInfo)
			: _info(&stdTypeInfo)
		{ }

		const char* GetRawName() const	{ return _info->name(); }

		std::string GetName() const {return "TypeInfo NOT IMPLEMENTED";}
		std::string GetNamespaceName() const {return "TypeInfo NOT IMPLEMENTED";}
		std::string GetClassName() const {return "TypeInfo NOT IMPLEMENTED";}

		std::string ToString() const {return "TypeInfo NOT IMPLEMENTED";}
	};

}

#endif
