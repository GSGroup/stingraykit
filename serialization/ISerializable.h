#ifndef STINGRAY_TOOLKIT_SERIALIZATION_ISERIALIZABLE_H
#define STINGRAY_TOOLKIT_SERIALIZATION_ISERIALIZABLE_H


#include <stingray/toolkit/shared_ptr.h>


namespace stingray
{

	/**
	 * @addtogroup core_settings
	 * @{
	 */

	class ObjectOStream;
	class ObjectIStream;

	struct ISerializable
	{
		virtual ~ISerializable() { }

		virtual void Serialize(ObjectOStream& ar) const = 0;
		virtual void Deserialize(ObjectIStream& ar) = 0;
	};
	STINGRAYKIT_DECLARE_PTR(ISerializable);


	namespace Detail
	{

		struct IObjectCache
		{
			virtual ~IObjectCache() { }

			virtual void Flush() = 0;
		};

	}


	typedef int PrimaryKeyType;

	struct IObjectSerializator
	{

		virtual ~IObjectSerializator() { }

		virtual PrimaryKeyType GetKey(const std::string& classname, const ISerializablePtr& value) const = 0;
		virtual ISerializablePtr GetObject(const std::string& classname, PrimaryKeyType key) const = 0;
		virtual void ReleaseRef(const std::string& classname, PrimaryKeyType key) const = 0;
	};

	/** @} */

}


#endif
