#ifndef STINGRAYKIT_SERIALIZATION_COMPOSITE_H
#define STINGRAYKIT_SERIALIZATION_COMPOSITE_H

// Copyright (c) 2011 - 2019, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/collection/IEnumerable.h>
#include <stingraykit/core/Final.h>
#include <stingraykit/serialization/Serializer.h>

namespace stingray
{

	/**
	* @addtogroup core_settings
	* @{
	*/

	/**
	 * @brief STINGRAYKIT COMPOSITE
	 * @par Example:
	 * @code
	 *
	 * Next declare:
	 *
	 *		STINGRAYKIT_DECLARE_COMPOSITE(CompositeClass,
	 *			STINGRAYKIT_DECLARE_COMPOSITE_MEMBER(int, SomeVal0, someval0),
	 *			STINGRAYKIT_DECLARE_COMPOSITE_MEMBER(int, SomeVal1, someval1, somenamespace1::SerializationTag),
	 *			STINGRAYKIT_DECLARE_COMPOSITE_MEMBER(int, SomeVal2, someval2),
	 *			STINGRAYKIT_DECLARE_COMPOSITE_MEMBER(int, SomeVal3, someval3),
	 *			STINGRAYKIT_DECLARE_COMPOSITE_MEMBER(int, SomeVal4, someval4, somenamespace2::SerializationTag)
	 *		);
	 *
	 *	turn into:
	 *
	 *		class CompositeClass
	 *			:	private virtual stingray::Final<CompositeClass>,
	 *				public virtual stingray::NonPolymorphicMarker
	 *		{
	 *		private:
	 *			int			_SomeVal0;
	 *			int			_SomeVal1;
	 *			int			_SomeVal2;
	 *			int			_SomeVal3;
	 *			int			_SomeVal4;
	 *
	 *		public:
	 *			CompositeClass(const int& SomeVal0, const int& SomeVal1, const int& SomeVal2, const int& SomeVal3, const int& SomeVal4)
	 *				:	_SomeVal0(SomeVal0),
	 *					_SomeVal1(SomeVal1),
	 *					_SomeVal2(SomeVal2),
	 *					_SomeVal3(SomeVal3),
	 *					_SomeVal4(SomeVal4)
	 *			{ }
	 *
	 *			CompositeClass() { }
	 *
	 *			int GetSomeVal0() const { return _SomeVal0; }
	 *
	 *			int GetSomeVal1() const { return _SomeVal1; }
	 *
	 *			int GetSomeVal2() const { return _SomeVal2; }
	 *
	 *			int GetSomeVal3() const { return _SomeVal3; }
	 *
	 *			int GetSomeVal4() const { return _SomeVal4; }
	 *
	 *			std::string ToString() const
	 *			{
	 *				return StringBuilder() % "{ "
	 *						% "SomeVal0" % ": " % _SomeVal0 % ", " %
	 *						% "SomeVal1" % ": " % _SomeVal1 % ", " %
	 *						% "SomeVal2" % ": " % _SomeVal2 % ", " %
	 *						% "SomeVal3" % ": " % _SomeVal3 % ", " %
	 *						% "SomeVal4" % ": " % _SomeVal4 % ", " % } ";
	 *			}
	 *
	 *			void Serialize(ObjectOStream& ar) const
	 *			{
	 *				ar.Serialize(key0, MakeSerializer<DefaultSerializationTag>(_SomeVal0));
	 *				ar.Serialize(key1, MakeSerializer<somenamespace1::SerializationTag>(_SomeVal1));
	 *				ar.Serialize(key2, MakeSerializer<DefaultSerializationTag>(_SomeVal2));
	 *				ar.Serialize(key3, MakeSerializer<DefaultSerializationTag>(_SomeVal3));
	 *				ar.Serialize(key4, MakeSerializer<somenamespace1::SerializationTag>(_SomeVal4));
	 *			}
	 *
	 *			void Deserialize(ObjectIStream& ar)
	 *			{
	 *				ar.Deserialize(key0, *MakeDeserializer<DefaultSerializationTag>(_SomeVal0));
	 *				ar.Deserialize(key1, *MakeDeserializer<somenamespace1::SerializationTag>(_SomeVal1));
	 *				ar.Deserialize(key2, *MakeDeserializer<DefaultSerializationTag>(_SomeVal2));
	 *				ar.Deserialize(key3, *MakeDeserializer<DefaultSerializationTag>(_SomeVal3));
	 *				ar.Deserialize(key4, *MakeDeserializer<somenamespace2::SerializationTag>(_SomeVal4));
	 *			}
	 *		};
	 *		typedef stingray::shared_ptr<CompositeClass> CompositeClassPtr;
	 *		typedef stingray::weak_ptr<CompositeClass> CompositeClassWeakPtr;
	 *
	 *		typedef stingray::IEnumerable<CompositeClassPtr>CompositeClassPtrEnumerable;
	 *		typedef stingray::shared_ptr<CompositeClassPtrEnumerable> CompositeClassPtrEnumerablePtr;
	 *		typedef stingray::weak_ptr<CompositeClassPtrEnumerable> CompositeClassPtrEnumerableWeakPtr;
	 *		typedef stingray::IEnumerator<CompositeClassPtr> CompositeClassPtrEnumerator;
	 *		typedef stingray::shared_ptr<CompositeClassPtrEnumerator>CompositeClassPtrEnumeratorPtr;
	 *		typedef stingray::weak_ptr<CompositeClassPtrEnumerator> CompositeClassPtrEnumeratorWeakPtr
	 *
	 *
	 * @endcode
	 */


#define STINGRAYKIT_DECLARE_COMPOSITE(ClassName, ...) \
		DETAIL_COMPOSITE_BEGIN(ClassName) \
		DETAIL_COMPOSITE_MEMBER_LIST(ClassName, __VA_ARGS__) \
		DETAIL_COMPOSITE_CTOR(ClassName, __VA_ARGS__) \
		DETAIL_COMPOSITE_TOSTRING(ClassName, __VA_ARGS__) \
		DETAIL_COMPOSITE_SERIALIZER(ClassName, __VA_ARGS__) \
		DETAIL_COMPOSITE_DESERIALIZER(ClassName, __VA_ARGS__) \
		DETAIL_COMPOSITE_END() \
		STINGRAYKIT_DECLARE_PTR(ClassName); \
		STINGRAYKIT_DECLARE_ENUMERABLE(ClassName##Ptr);


#define STINGRAYKIT_DECLARE_COMPOSITE_MEMBER_IMPL_3(MemberType, MemberName, SerializationKey) \
		MK_PARAM(MemberType, MemberName, SerializationKey, DefaultSerializationTag, DefaultSerializationTag)

#define STINGRAYKIT_DECLARE_COMPOSITE_MEMBER_IMPL_4(MemberType, MemberName, SerializationKey, SerializationTag) \
		MK_PARAM(MemberType, MemberName, SerializationKey, SerializationTag, SerializationTag)

#define STINGRAYKIT_DECLARE_COMPOSITE_MEMBER(...) \
		STINGRAYKIT_CAT(STINGRAYKIT_DECLARE_COMPOSITE_MEMBER_IMPL_,STINGRAYKIT_NARGS(__VA_ARGS__))(__VA_ARGS__)


#define DETAIL_METHOD_NUMBER_IMPL_5 1
#define DETAIL_METHOD_NUMBER_IMPL_10 2
#define DETAIL_METHOD_NUMBER_IMPL_15 3
#define DETAIL_METHOD_NUMBER_IMPL_20 4
#define DETAIL_METHOD_NUMBER_IMPL_25 5
#define DETAIL_METHOD_NUMBER_IMPL_30 6
#define DETAIL_METHOD_NUMBER_IMPL_35 7
#define DETAIL_METHOD_NUMBER_IMPL_40 8
#define DETAIL_METHOD_NUMBER_IMPL_45 9
#define DETAIL_METHOD_NUMBER_IMPL_50 10
#define DETAIL_METHOD_NUMBER_IMPL_55 11
#define DETAIL_METHOD_NUMBER_IMPL_60 12
#define DETAIL_METHOD_NUMBER_IMPL_65 13
#define DETAIL_METHOD_NUMBER_IMPL_70 14
#define DETAIL_METHOD_NUMBER_IMPL_75 15
#define DETAIL_METHOD_NUMBER_IMPL_80 16
#define DETAIL_METHOD_NUMBER_IMPL_85 17
#define DETAIL_METHOD_NUMBER_IMPL_90 18
#define DETAIL_METHOD_NUMBER_IMPL_95 19
#define DETAIL_METHOD_NUMBER_IMPL_100 20


#define DETAIL_METHOD_NUMBER(...) STINGRAYKIT_CAT(DETAIL_METHOD_NUMBER_IMPL_, STINGRAYKIT_NARGS(__VA_ARGS__))

#define DETAIL_MEMBER_NARG 5

#define DETAIL_MEMBER_TYPE_NTH_METHOD(N, ...)			STINGRAYKIT_NTH_ELEMENT(STINGRAYKIT_ADD(STINGRAYKIT_MUL(DETAIL_MEMBER_NARG, N), 0), __VA_ARGS__)
#define DETAIL_MEMBER_NAME_NTH_METHOD(N, ...)			STINGRAYKIT_NTH_ELEMENT(STINGRAYKIT_ADD(STINGRAYKIT_MUL(DETAIL_MEMBER_NARG, N), 1), __VA_ARGS__)
#define DETAIL_SERIALIZATION_KEY_NTH_METHOD(N, ...)		STINGRAYKIT_NTH_ELEMENT(STINGRAYKIT_ADD(STINGRAYKIT_MUL(DETAIL_MEMBER_NARG, N), 2), __VA_ARGS__)
#define DETAIL_SERIALIZATION_TAG_NTH_METHOD(N, ...)		STINGRAYKIT_NTH_ELEMENT(STINGRAYKIT_ADD(STINGRAYKIT_MUL(DETAIL_MEMBER_NARG, N), 3), __VA_ARGS__)
#define DETAIL_DESERIALIZATION_TAG_NTH_METHOD(N, ...)	STINGRAYKIT_NTH_ELEMENT(STINGRAYKIT_ADD(STINGRAYKIT_MUL(DETAIL_MEMBER_NARG, N), 4), __VA_ARGS__)


#define DETAIL_COMPOSITE_BEGIN(ClassName) class ClassName : STINGRAYKIT_NONPOLYMORPHIC(ClassName) {

#define DETAIL_COMPOSITE_END()  };


#define DETAIL_COMPOSITE_CTOR_MEMBER_INITIALIZATION(N, ...) \
		STINGRAYKIT_COMMA_IF(N) STINGRAYKIT_CAT(_, DETAIL_MEMBER_NAME_NTH_METHOD(N, __VA_ARGS__))(DETAIL_MEMBER_NAME_NTH_METHOD(N, __VA_ARGS__))

#define DETAIL_COMPOSITE_CTOR_PARAM(N, ...) \
		STINGRAYKIT_COMMA_IF(N) const DETAIL_MEMBER_TYPE_NTH_METHOD(N, __VA_ARGS__)& DETAIL_MEMBER_NAME_NTH_METHOD(N, __VA_ARGS__)

#define DETAIL_COMPOSITE_CTOR(ClassName, ...) \
		public: ClassName(STINGRAYKIT_REPEAT(DETAIL_METHOD_NUMBER(__VA_ARGS__), DETAIL_COMPOSITE_CTOR_PARAM, __VA_ARGS__)) : \
		STINGRAYKIT_REPEAT(DETAIL_METHOD_NUMBER(__VA_ARGS__), DETAIL_COMPOSITE_CTOR_MEMBER_INITIALIZATION, __VA_ARGS__) { } \
		ClassName() { }


#define DETAIL_COMPOSITE_MEMBER_LIST(ClassName, ...) \
		STINGRAYKIT_REPEAT(DETAIL_METHOD_NUMBER(__VA_ARGS__), DETAIL_COMPOSITE_MEMBER, ClassName, __VA_ARGS__)

#define DETAIL_COMPOSITE_MEMBER(N, ClassName, ...) \
		DETAIL_COMPOSITE_MEMBER_IMPL(DETAIL_MEMBER_TYPE_NTH_METHOD(N, __VA_ARGS__), DETAIL_MEMBER_NAME_NTH_METHOD(N, __VA_ARGS__))

#define DETAIL_COMPOSITE_MEMBER_IMPL(MemberType, MemberName) \
		private: MemberType	STINGRAYKIT_CAT(_, MemberName); public: MemberType STINGRAYKIT_CAT(Get, MemberName)() const { return STINGRAYKIT_CAT(_, MemberName); }


#define DETAIL_COMPOSITE_TOSTRING(ClassName, ...) \
		public: std::string ToString() const { return StringBuilder() % "{ " % STINGRAYKIT_REPEAT(DETAIL_METHOD_NUMBER(__VA_ARGS__), DETAIL_COMPOSITE_TOSTRING_IMPL, __VA_ARGS__) % " }"; }

#define DETAIL_COMPOSITE_TOSTRING_IMPL(N, ...) \
		DETAIL_ESCAPED_COMMA_IF(N) TO_STRING(DETAIL_MEMBER_NAME_NTH_METHOD(N, __VA_ARGS__)) % ": " % stingray::ToString(STINGRAYKIT_CAT(_, DETAIL_MEMBER_NAME_NTH_METHOD(N, __VA_ARGS__)))

#define DETAIL_ESCAPED_COMMA_IF(A) STINGRAYKIT_CAT(DETAIL_ESCAPED_COMMA_IF_, STINGRAYKIT_BOOL(A))

#define DETAIL_ESCAPED_COMMA_IF_0
#define DETAIL_ESCAPED_COMMA_IF_1 % ", " %


#define DETAIL_COMPOSITE_SERIALIZER(ClassName, ...) \
		template <typename StreamType> void Serialize(StreamType& ar) const { STINGRAYKIT_REPEAT(DETAIL_METHOD_NUMBER(__VA_ARGS__), DETAIL_COMPOSITE_SERIALIZER_IMPL, __VA_ARGS__) }

#define DETAIL_COMPOSITE_SERIALIZER_IMPL(N, ...) \
		ar.Serialize(TO_STRING(DETAIL_SERIALIZATION_KEY_NTH_METHOD(N, __VA_ARGS__)), MakeSerializer<DETAIL_SERIALIZATION_TAG_NTH_METHOD(N, __VA_ARGS__)>(STINGRAYKIT_CAT(_, DETAIL_MEMBER_NAME_NTH_METHOD(N, __VA_ARGS__))));


#define DETAIL_COMPOSITE_DESERIALIZER(ClassName, ...) \
		template <typename StreamType> void Deserialize(StreamType& ar) { STINGRAYKIT_REPEAT(DETAIL_METHOD_NUMBER(__VA_ARGS__), DETAIL_COMPOSITE_DESERIALIZER_IMPL, __VA_ARGS__) }

#define DETAIL_COMPOSITE_DESERIALIZER_IMPL(N, ...) \
		ar.Deserialize(TO_STRING(DETAIL_SERIALIZATION_KEY_NTH_METHOD(N, __VA_ARGS__)), *MakeDeserializer<DETAIL_DESERIALIZATION_TAG_NTH_METHOD(N, __VA_ARGS__)>(STINGRAYKIT_CAT(_, DETAIL_MEMBER_NAME_NTH_METHOD(N, __VA_ARGS__))));

}

#endif
