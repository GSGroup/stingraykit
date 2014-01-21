#ifndef STINGRAY_TOOLKIT_TYPEINFO_H
#define STINGRAY_TOOLKIT_TYPEINFO_H


#include <string>
#include <typeinfo>

#include <stingray/toolkit/exception.h>


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

		bool operator == (const TypeInfo& other) const
		{ return *TOOLKIT_REQUIRE_NOT_NULL(_info) == *TOOLKIT_REQUIRE_NOT_NULL(other._info); }

		const char* GetRawName() const	{ return _info->name(); }

		std::string GetName() const;
		std::string GetNamespaceName() const;
		std::string GetClassName() const;

		std::string ToString() const;
	};

}

#endif
