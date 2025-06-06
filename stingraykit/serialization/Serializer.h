#ifndef STINGRAYKIT_SERIALIZATION_SERIALIZER_H
#define STINGRAYKIT_SERIALIZATION_SERIALIZER_H

// Copyright (c) 2011 - 2025, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/optional.h>

namespace stingray
{

	/**
	 * @addtogroup core_settings
	 * @{
	 */

	namespace SerializerUtils
	{

		STINGRAYKIT_DECLARE_METHOD_CHECK(Serialize);
		STINGRAYKIT_DECLARE_METHOD_CHECK(SerializeAsValue);

		STINGRAYKIT_DECLARE_METHOD_CHECK(Deserialize);
		STINGRAYKIT_DECLARE_METHOD_CHECK(DeserializeAsValue);

	}


	template < typename Tag, typename T, typename Enabler = void >
	struct Serialization { };


	template < typename Tag, typename T, typename Enabler = void >
	struct Serializer;

	template < typename Tag, typename T >
	struct Serializer<Tag, T, typename EnableIf<!IsOptional<T>::Value && SerializerUtils::HasMethod_Serialize<Serialization<Tag, T, void> >::Value, void>::ValueT>
	{
		const T&	Object;

		explicit Serializer(const T& object) : Object(object) { }

		template < typename OStream_ >
		void Serialize(OStream_& ar) const
		{ Serialization<Tag, T, void>::Serialize(ar, Object); }
	};

	template < typename Tag, typename T >
	struct Serializer<Tag, T, typename EnableIf<!IsOptional<T>::Value && SerializerUtils::HasMethod_SerializeAsValue<Serialization<Tag, T, void> >::Value, void>::ValueT>
	{
		const T&	Object;

		explicit Serializer(const T& object) : Object(object) { }

		template < typename OStream_ >
		void SerializeAsValue(OStream_& ar) const
		{ Serialization<Tag, T, void>::SerializeAsValue(ar, Object); }
	};

	template < typename Tag, typename T >
	struct Serializer<Tag, T, typename EnableIf<!IsOptional<T>::Value
			&& !SerializerUtils::HasMethod_Serialize<Serialization<Tag, T, void> >::Value
			&& !SerializerUtils::HasMethod_SerializeAsValue<Serialization<Tag, T, void> >::Value, void>::ValueT>
	{
		const T&	Object;

		explicit Serializer(const T& object) : Object(object) { }
	};

	template < typename Tag, typename T >
	struct Serializer<Tag, optional<T>, void >
	{
		const T&		Object;

		explicit Serializer(const T& object) : Object(object) { }

		template < typename OStream_ >
		void SerializeAsValue(OStream_& ar) const
		{ ar.Serialize(Serializer<Tag, T>(Object)); }
	};


	namespace Detail
	{

		template < typename Tag, typename T >
		struct SerializerCreator
		{
			static optional<Serializer<Tag, T> > Create(const T& object)
			{ return Serializer<Tag, T>(object); }
		};

		template < typename Tag, typename T >
		struct SerializerCreator<Tag, optional<T> >
		{
			static optional<Serializer<Tag, optional<T> > > Create(const optional<T>& object)
			{
				if (!object)
					return null;
				return Serializer<Tag, optional<T> >(*object);
			}
		};

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

			template < typename IStream_ >
			void DeserializeAsValue(IStream_& ar)
			{
				T object;
				ar.Deserialize(*Deserializer_(object));
				Object = object;
			}
		};

	}


	template < typename Tag, typename T >
	optional<Serializer<Tag, T> > MakeSerializer(const T& object)
	{ return Detail::SerializerCreator<Tag, T>::Create(object); }


	template < typename Tag, typename T, typename Enabler = void >
	struct Deserializer;

	template < typename Tag, typename T >
	struct Deserializer<Tag, T, typename EnableIf<!IsOptional<T>::Value && SerializerUtils::HasMethod_Deserialize<Serialization<Tag, T, void> >::Value, void>::ValueT>
	{
		T&		Object;

		explicit Deserializer(T& object) : Object(object) { }

		template < typename IStream_ >
		void Deserialize(IStream_& ar)
		{ Serialization<Tag, T, void>::Deserialize(ar, Object); }

		Deserializer& operator * () { return *this; }

		Deserializer& operator = (const T& object)
		{
			Object = object;
			return *this;
		}
	};

	template < typename Tag, typename T >
	struct Deserializer<Tag, T, typename EnableIf<!IsOptional<T>::Value && SerializerUtils::HasMethod_DeserializeAsValue<Serialization<Tag, T, void> >::Value, void>::ValueT>
	{
		T&		Object;

		explicit Deserializer(T& object) : Object(object) { }

		template < typename IStream_ >
		void DeserializeAsValue(IStream_& ar)
		{ Serialization<Tag, T, void>::DeserializeAsValue(ar, Object); }

		Deserializer& operator * () { return *this; }

		Deserializer& operator = (const T& object)
		{
			Object = object;
			return *this;
		}
	};

	template < typename Tag, typename T >
	struct Deserializer<Tag, T, typename EnableIf<!IsOptional<T>::Value
			&& !SerializerUtils::HasMethod_Deserialize<Serialization<Tag, T, void> >::Value
			&& !SerializerUtils::HasMethod_DeserializeAsValue<Serialization<Tag, T, void> >::Value, void>::ValueT>
	{
		T&		Object;

		explicit Deserializer(T& object) : Object(object) { }

		Deserializer& operator * () { return *this; }
	};

	template < typename Tag, typename T >
	struct Deserializer<Tag, optional<T>, void>
	{
	private:
		typedef Detail::OptionalDeserializationHelper<T, Deserializer<Tag, T> > DeserializationHelper;

		optional<DeserializationHelper>		_helper;

	public:
		explicit Deserializer(optional<T>& object) : _helper(DeserializationHelper(object)) { }

		optional<DeserializationHelper>& operator * () { return _helper; }
	};

	template < typename Tag, typename T >
	Deserializer<Tag, T> MakeDeserializer(T& object) { return Deserializer<Tag, T>(object); }


	struct DefaultSerializationTag;

	template < typename Object >
	struct Serialization<DefaultSerializationTag, Object, void>
	{
		template < typename OStream >
		static void SerializeAsValue(OStream& ar, const Object& object)
		{ ar.Serialize(object); }

		template < typename IStream >
		static void DeserializeAsValue(IStream& ar, Object& object)
		{ ar.Deserialize(object); }
	};

	/** @} */

}

#endif
