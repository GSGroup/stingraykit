#ifndef STINGRAYKIT_SERIALIZATION_ISERIALIZABLE_H
#define STINGRAYKIT_SERIALIZATION_ISERIALIZABLE_H

// Copyright (c) 2011 - 2015, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.


#include <stingraykit/shared_ptr.h>


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
