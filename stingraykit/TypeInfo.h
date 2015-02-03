#ifndef STINGRAYKIT_TYPEINFO_H
#define STINGRAYKIT_TYPEINFO_H

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

		bool operator == (const TypeInfo& other) const
		{ return *_info == *other._info; }

		bool operator < (const TypeInfo& other) const
		{ return _info->before(*other._info); }

		const char* GetRawName() const	{ return _info->name(); }

		std::string GetName() const;
		std::string GetNamespaceName() const;
		std::string GetClassName() const;

		std::string ToString() const;
	};

}

#endif
