// Copyright (c) 2011 - 2024, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#ifndef STINGRAYKIT_SERIALIZATION_SERIALIZATIONHELPER_H
#define STINGRAYKIT_SERIALIZATION_SERIALIZATIONHELPER_H

#include <stingraykit/serialization/Serialization.h>

namespace stingray
{

	namespace Detail
	{

		class PairCreator
		{
		private:
			std::string	_key;

		public:
			explicit PairCreator(const std::string& key) : _key(key) { }

			template < typename T >
			operator std::pair<std::string, T> () const { return std::make_pair(_key, T()); }
		};

		template < typename ValueType >
		struct ToPairType
		{ using ValueT = std::pair<std::string, ValueType>; };

		template < typename Pair >
		auto TransformPair(const Pair& pair)
		{ return std::pair<std::string, typename Pair::second_type>(pair.first, pair.second); }

	}


	template < typename... Types >
	class SerializationHelper
	{
		using TupleType = Tuple<typename TypeListTransform<TypeList<Types...>, Detail::ToPairType>::ValueT>;

		template < size_t Index >
		struct SerializeTuple
		{
			static void Call(ObjectOStream& ar, const TupleType& tuple)
			{
				const auto& pair = tuple.template Get<Index>();
				ar.Serialize(pair.first, pair.second);
			}
		};

		template < size_t Index >
		struct DeserializeTuple
		{
			static void Call(ObjectIStream& ar, TupleType& tuple)
			{
				auto& pair = tuple.template Get<Index>();
				ar.Deserialize(pair.first, pair.second);
			}
		};

	private:
		TupleType	_tuple;

	public:
		explicit SerializationHelper(const std::pair<std::string, Types>&... pairs)
			: _tuple(pairs...)
		{ }

		template < typename... Keys >
		explicit SerializationHelper(const Keys&... keys)
			: _tuple(Detail::PairCreator(keys)...)
		{ }

		template < size_t Index = 0 >
		auto GetValue() const
		{ return _tuple.template Get<Index>().second; }

		void Serialize(ObjectOStream& ar) const
		{ For<TupleType::Size, SerializationHelper::SerializeTuple>::Do(wrap_ref(ar), _tuple); }

		void Deserialize(ObjectIStream& ar)
		{ For<TupleType::Size, SerializationHelper::DeserializeTuple>::Do(wrap_ref(ar), wrap_ref(_tuple)); }

		SerializationHelper& operator * ()
		{ return *this; }
	};


	template < typename T >
	auto MakeSerializationHelper(const std::string& key, const T& value)
	{ return SerializationHelper<T>(std::make_pair(key, value)); }


	template < typename... Keys,  typename... Ts >
	auto MakeSerializationHelper(const std::pair<Keys, Ts>&... pairs)
	{ return SerializationHelper<Ts...>(Detail::TransformPair(pairs)...); }

}

#endif
