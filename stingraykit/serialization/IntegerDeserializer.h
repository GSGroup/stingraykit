#ifndef STINGRAYKIT_SERIALIZATION_INTEGERDESERIALIZER_H
#define STINGRAYKIT_SERIALIZATION_INTEGERDESERIALIZER_H

#include <stingraykit/serialization/Serializer.h>

namespace stingray
{

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
	struct Serialization<IntegerSerializationTag, Integer_>
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
	struct Serialization<IntegerSerializationTag, s64>
	{
		template < typename IStream_ >
		static void DeserializeAsValue(IStream_& ar, s64& integer)
		{ ar.Deserialize(integer); }
	};


	template < >
	struct Serialization<IntegerSerializationTag, u64>
	{
		template < typename IStream_ >
		static void DeserializeAsValue(IStream_& ar, u64& integer)
		{ ar.Deserialize(integer); }
	};


	template < typename Integer_ >
	Deserializer<IntegerSerializationTag, Integer_> MakeIntegerDeserializer(Integer_& integer)
	{ return MakeDeserializer<IntegerSerializationTag>(integer); }

}

#endif
