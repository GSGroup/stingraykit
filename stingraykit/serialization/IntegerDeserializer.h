#ifndef STINGRAYKIT_SERIALIZATION_INTEGERDESERIALIZER_H
#define STINGRAYKIT_SERIALIZATION_INTEGERDESERIALIZER_H

// Copyright (c) 2011 - 2019, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/serialization/Serializer.h>

#include <limits>

namespace stingray
{

	/**
	 * @addtogroup core_settings
	 * @{
	 */

	namespace Detail
	{

		template < typename Integer_ >
		struct CheckedIntegerCaster
		{
			template < typename Container_ >
			Integer_ operator()(Container_ contatiner) const
			{
				const Container_ begin = static_cast<Container_>(std::numeric_limits<Integer_>::min());
				const Container_ end = static_cast<Container_>(std::numeric_limits<Integer_>::max()) + 1;
				STINGRAYKIT_CHECK_RANGE(contatiner, begin, end);
				return static_cast<Integer_>(contatiner);
			}
		};

	}


	struct IntegerSerializationTag;


	template < typename Integer_ >
	struct Serialization<IntegerSerializationTag, Integer_, void>
	{
		typedef s64 Container;

		template < typename IStream_ >
		static void DeserializeAsValue(IStream_& ar, Integer_& integer)
		{
			Container container;
			ar.Deserialize(container);
			integer = Detail::CheckedIntegerCaster<Integer_>()(container);
		}
	};


	template < >
	struct Serialization<IntegerSerializationTag, s64, void>
	{
		template < typename IStream_ >
		static void DeserializeAsValue(IStream_& ar, s64& integer)
		{ ar.Deserialize(integer); }
	};


	template < >
	struct Serialization<IntegerSerializationTag, u64, void>
	{
		template < typename IStream_ >
		static void DeserializeAsValue(IStream_& ar, u64& integer)
		{ ar.Deserialize(integer); }
	};


	template < typename Integer_ >
	Deserializer<IntegerSerializationTag, Integer_> MakeIntegerDeserializer(Integer_& integer)
	{ return MakeDeserializer<IntegerSerializationTag>(integer); }

	/** @} */

}

#endif
