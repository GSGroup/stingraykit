#ifndef STINGRAYKIT_SERIALIZATION_STRINGPROXYSERIALIZATION_H
#define STINGRAYKIT_SERIALIZATION_STRINGPROXYSERIALIZATION_H

// Copyright (c) 2011 - 2019, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/serialization/Serializer.h>
#include <stingraykit/string/ToString.h>

namespace stingray
{

	/**
	 * @addtogroup core_settings
	 * @{
	 */

	struct StringProxySerializationTag;


	template < typename T >
	struct Serialization<StringProxySerializationTag, T, void>
	{
		template < typename OStream_ >
		static void SerializeAsValue(OStream_& ar, const T& object)
		{ ar.Serialize(ToString(object)); }

		template < typename IStream_ >
		static void DeserializeAsValue(IStream_& ar, T& object)
		{
			std::string tmp;
			ar.Deserialize(tmp);
			object = FromString<T>(tmp);
		}
	};

	template < typename T >
	optional<Serializer<StringProxySerializationTag, T> > MakeStringProxySerializer(const T& object)
	{ return MakeSerializer<StringProxySerializationTag, T>(object); };


	template < typename T >
	Deserializer<StringProxySerializationTag, T> MakeStringProxyDeserializer(T& object)
	{ return MakeDeserializer<StringProxySerializationTag, T>(object); };

	/** @} */

}

#endif
