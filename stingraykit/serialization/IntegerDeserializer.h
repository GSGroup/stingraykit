#ifndef STINGRAYKIT_SERIALIZATION_INTEGERDESERIALIZER_H
#define STINGRAYKIT_SERIALIZATION_INTEGERDESERIALIZER_H

#include <stingraykit/exception.h>

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


		template < typename Wrapped_ >
		struct IntegerDeserializerBase
		{
			Wrapped_ Wrapped;

		public:
			IntegerDeserializerBase()
				:	Wrapped()
			{ }

			operator Wrapped_() const { return Wrapped; }
		};


		template < typename Integer_ >
		struct CheckedIntegerDeserializer : public IntegerDeserializerBase<Integer_>
		{
			typedef s64 Container;

			template < typename OStream_ >
			void Deserialize(OStream_& ar)
			{
				Container container;
				ar.Deserialize(container);
				this->Wrapped = CheckedIntegerCaster<Integer_>()(container);
			}
		};


		template < typename Integer_ >
		struct SimpleIntegerDeserializer : public IntegerDeserializerBase<Integer_>
		{
			template < typename OStream_ >
			void Deserialize(OStream_& ar)
			{ ar.Deserialize(this->Wrapped); }
		};

	}


	template < typename Integer_ >
	struct IntegerDeserializer : public Detail::CheckedIntegerDeserializer<Integer_>
	{ };


	template < >
	struct IntegerDeserializer<s64> : public Detail::SimpleIntegerDeserializer<s64>
	{ };


	template < >
	struct IntegerDeserializer<u64> : public Detail::SimpleIntegerDeserializer<u64>
	{ };

}

#endif
