#ifndef STINGRAYKIT_SERIALIZATION_SERIALIZER_H
#define STINGRAYKIT_SERIALIZATION_SERIALIZER_H

// Copyright (c) 2011 - 2017, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/serialization/Serialization.h>

namespace stingray
{

	/**
	 * @addtogroup core_settings
	 * @{
	 */

	template < typename SerializationTag, typename T >
	struct Serializer
	{
		const T		Object;

		explicit Serializer(const T& object) : Object(object) { }
	};

	template < typename SerializationTag, typename T >
	struct Serializer<SerializationTag, optional<T> >
	{
		optional<T>		Object;

		explicit Serializer(const optional<T>& object) : Object(object) { }

		void SerializeAsValue(ObjectOStream& ar) const
		{
			if (Object)
				ar.Serialize(Serializer<SerializationTag, T>(*Object));
			else
				ar.Serialize(null);
		}
	};

	template < typename SerializationTag, typename T >
	Serializer<SerializationTag, T> MakeSerializer(const T& object) { return Serializer<SerializationTag, T>(object); }


	namespace Detail
	{

		template < typename T, typename Deserializer_ >
		struct OptionalDeserializationHelper
		{
			optional<T>		Object;
			optional<T>&	Target;

			OptionalDeserializationHelper() : Object(), Target(Object) { }

			explicit OptionalDeserializationHelper(optional<T>& target) : Object(), Target(target) { }

			const OptionalDeserializationHelper& operator = (const OptionalDeserializationHelper& other) const
			{
				Target = other.Object;
				return *this;
			}

			void DeserializeAsValue(ObjectIStream& ar)
			{
				T object;
				ar.Deserialize(Deserializer_(object).GetReference());
				Object = object;
			}
		};

	}


	template < typename SerializationTag, typename T >
	struct Deserializer
	{
		T&		Object;

		explicit Deserializer(T& object) : Object(object) { }

		Deserializer& GetReference() { return *this; }
	};

	template < typename SerializationTag, typename T >
	class Deserializer<SerializationTag, optional<T> >
	{
		typedef Detail::OptionalDeserializationHelper<T, Deserializer<SerializationTag, T> > DeserializationHelper;

		optional<DeserializationHelper>		_helper;

	public:
		explicit Deserializer(optional<T>& object) : _helper(DeserializationHelper(object)) { }

		optional<DeserializationHelper>& GetReference() { return _helper; }
	};

	template < typename SerializationTag, typename T >
	Deserializer<SerializationTag, T> MakeDeserializer(T& object) { return Deserializer<SerializationTag, T>(object); }

	/** @} */

}

#endif
