#ifndef STINGRAYKIT_ANY_H
#define STINGRAYKIT_ANY_H

// Copyright (c) 2011 - 2025, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/serialization/ISerializable.h>
#include <stingraykit/string/ToString.h>
#include <stingraykit/factory/FactoryPublic.h>

namespace stingray
{

	namespace Detail {
	namespace any
	{
		struct AnyType
		{
			STINGRAYKIT_ENUM_VALUES
			(
				Empty,
				Bool,
				Char,
				UChar,
				Short,
				UShort,
				Int,
				UInt,
				Long,
				ULong,
				LongLong,
				ULongLong,
				Float,
				Double,
				String,
				Object,
				SerializableObject
			);
			STINGRAYKIT_DECLARE_ENUM_CLASS(AnyType);
		};


		struct IObjectHolder : public virtual ISerializable, public virtual IFactoryObject
		{
			~IObjectHolder() override { }

			virtual IObjectHolder* Clone() const = 0;
			virtual std::string ToString() const = 0;
			virtual bool IsSerializable() const = 0;
		};


		template < typename T, bool IsStringRepresentable_ = IsStringRepresentable<T>::Value >
		struct ObjectToString
		{ static std::string ToString(const T& obj) { return "<not a string-representable type>"; } };

		template < >
		struct ObjectToString<ISerializablePtr, true>
		{ static std::string ToString(const ISerializablePtr& obj) { return "<a class derived from ISerializable>"; } };

		template < typename T >
		struct ObjectToString<T, true>
		{ static std::string ToString(const T& obj) { return stingray::ToString(obj); } };


		template < typename T >
		struct ObjectHolder;

		template < typename T >
		struct ObjectHolderBase : public virtual IObjectHolder
		{
			T	Object;

			ObjectHolderBase() : Object() { }
			ObjectHolderBase(const T& object) : Object(object) { }
			ObjectHolderBase(T&& object) : Object(std::move(object)) { }

			IObjectHolder* Clone() const override				{ return new ObjectHolder<T>(Object); }
			std::string ToString() const override				{ return ObjectToString<T>::ToString(Object); }
		};

		template < typename T >
		struct ObjectHolder : public ObjectHolderBase<T>
		{
			ObjectHolder(const T& object) : ObjectHolderBase<T>(object) { }
			ObjectHolder(T&& object) : ObjectHolderBase<T>(std::move(object)) { }

			void Serialize(ObjectOStream& ar) const override		{ STINGRAYKIT_THROW(NotSupportedException()); }
			void Deserialize(ObjectIStream& ar) override			{ STINGRAYKIT_THROW(NotSupportedException()); }
			bool IsSerializable() const override					{ return false; }

			std::string GetClassName() const override { STINGRAYKIT_THROW(NotImplementedException()); }
		};

		template < >
		struct ObjectHolder<ISerializablePtr> : public ObjectHolderBase<ISerializablePtr>
		{
			ObjectHolder() { }
			ObjectHolder(const ISerializablePtr& object) : ObjectHolderBase<ISerializablePtr>(object) { }
			ObjectHolder(ISerializablePtr&& object) : ObjectHolderBase<ISerializablePtr>(std::move(object)) { }

			void Serialize(ObjectOStream& ar) const override;
			void Deserialize(ObjectIStream& ar) override;
			bool IsSerializable() const override					{ return true; }

			STINGRAYKIT_REGISTER_CLASS(ObjectHolder<ISerializablePtr>);
		};

#define STINGRAYKIT_DECLARE_SERIALIZABLE_ANY_OBJECTHOLDER(...) \
		namespace stingray { \
		namespace Detail { \
		namespace any { \
			template < > \
			struct ObjectHolder<__VA_ARGS__> : public ObjectHolderBase<__VA_ARGS__> \
			{ \
				ObjectHolder() { } \
				ObjectHolder(const __VA_ARGS__& object) : ObjectHolderBase<__VA_ARGS__>(object) { } \
				ObjectHolder(__VA_ARGS__&& object) : ObjectHolderBase<__VA_ARGS__>(std::move(object)) { } \
				void Serialize(ObjectOStream& ar) const override; \
				void Deserialize(ObjectIStream& ar) override; \
				bool IsSerializable() const override	{ return true; } \
				STINGRAYKIT_REGISTER_CLASS(ObjectHolder<__VA_ARGS__>); \
			}; \
		}}}

#define STINGRAYKIT_DEFINE_SERIALIZABLE_ANY_OBJECTHOLDER(...) \
		namespace stingray { \
		namespace Detail { \
		namespace any { \
				void ObjectHolder<__VA_ARGS__>::Serialize(ObjectOStream& ar) const	{ ar.Serialize("obj", Object); } \
				void ObjectHolder<__VA_ARGS__>::Deserialize(ObjectIStream& ar)		{ ar.Deserialize("obj", Object); } \
		}}}

		union DataType
		{
			bool					Bool;
			char					Char;
			unsigned char			UChar;
			short					Short;
			unsigned short			UShort;
			int						Int;
			unsigned int			UInt;
			long					Long;
			unsigned long			ULong;
			long long				LongLong;
			unsigned long long		ULongLong;
			float					Float;
			double					Double;
			StorageFor<std::string>	String;
			IObjectHolder*			Object;
		};

		template < typename T, bool IsPtr = IsSharedPtr<T>::Value >
		struct IsPtrToISerializable : FalseType { };

		template < typename T >
		struct IsPtrToISerializable<T, true> : IsInherited<typename GetSharedPtrParam<T>::ValueT, ISerializable> { };

		template < typename T, bool PtrToISerializable = IsPtrToISerializable<T>::Value >
		struct CppTypeToAnyUnionType
		{ static const AnyType::Enum Value = AnyType::Object; };

		template < typename T >
		struct CppTypeToAnyUnionType<T, true>
		{ static const AnyType::Enum Value = AnyType::SerializableObject; };

#define CPPTYPE_TO_ANYUNIONTYPE(Type_, EnumVal_) template < bool PtrToISerializable > struct CppTypeToAnyUnionType<Type_, PtrToISerializable> { static const AnyType::Enum Value = AnyType::EnumVal_; }
		CPPTYPE_TO_ANYUNIONTYPE( bool,					Bool );
		CPPTYPE_TO_ANYUNIONTYPE( char,					Char );
		CPPTYPE_TO_ANYUNIONTYPE( unsigned char,			UChar );
		CPPTYPE_TO_ANYUNIONTYPE( short,					Short );
		CPPTYPE_TO_ANYUNIONTYPE( unsigned short,		UShort );
		CPPTYPE_TO_ANYUNIONTYPE( int,					Int );
		CPPTYPE_TO_ANYUNIONTYPE( unsigned int,			UInt );
		CPPTYPE_TO_ANYUNIONTYPE( long,					Long );
		CPPTYPE_TO_ANYUNIONTYPE( unsigned long,			ULong );
		CPPTYPE_TO_ANYUNIONTYPE( long long,				LongLong );
		CPPTYPE_TO_ANYUNIONTYPE( unsigned long long,	ULongLong );
		CPPTYPE_TO_ANYUNIONTYPE( float,					Float );
		CPPTYPE_TO_ANYUNIONTYPE( double,				Double );
		CPPTYPE_TO_ANYUNIONTYPE( std::string,			String );
#undef CPPTYPE_TO_ANYUNIONTYPE

		template < AnyType::Enum Type_ >
		struct AnyValAccessor;

#define ANY_VAL_ACCESSOR(EnumVal_, Set_, Get_) \
		template < > struct AnyValAccessor<AnyType::EnumVal_> \
		{ \
			template < typename T > \
			static void Set(DataType& data, T&& val) \
			{ \
				typedef typename Decay<T>::ValueT RawType; \
				Set_; \
			} \
			template < typename T > \
			static const T* Get(AnyType type, const DataType& data) \
			{ \
				if (type != CppTypeToAnyUnionType<T>::Value) \
					return NULL; \
				Get_; \
			} \
			template < typename T > \
			static T* Get(AnyType type, DataType& data) \
			{ \
				if (type != CppTypeToAnyUnionType<T>::Value) \
					return NULL; \
				Get_; \
			} \
		}

		/*				  Type		Set										Get					*/
		ANY_VAL_ACCESSOR( Empty, 	/*empty*/,								return NULL; );
		ANY_VAL_ACCESSOR( Bool,		data.Bool	= val,						return &data.Bool );
		ANY_VAL_ACCESSOR( Char,		data.Char	= val,						return &data.Char );
		ANY_VAL_ACCESSOR( UChar,	data.UChar	= val,						return &data.UChar );
		ANY_VAL_ACCESSOR( Short,	data.Short	= val,						return &data.Short );
		ANY_VAL_ACCESSOR( UShort,	data.UShort	= val,						return &data.UShort );
		ANY_VAL_ACCESSOR( Int,		data.Int	= val,						return &data.Int );
		ANY_VAL_ACCESSOR( UInt,		data.UInt	= val,						return &data.UInt );
		ANY_VAL_ACCESSOR( Long,		data.Long	= val,						return &data.Long );
		ANY_VAL_ACCESSOR( ULong,	data.ULong	= val,						return &data.ULong );
		ANY_VAL_ACCESSOR( LongLong,	data.LongLong	= val,					return &data.LongLong );
		ANY_VAL_ACCESSOR( ULongLong,data.ULongLong	= val,					return &data.ULongLong );
		ANY_VAL_ACCESSOR( Float,	data.Float	= val,						return &data.Float );
		ANY_VAL_ACCESSOR( Double,	data.Double	= val,						return &data.Double );
		ANY_VAL_ACCESSOR( String,	data.String.Ctor(std::forward<T>(val)),	return &data.String.Ref() );

		ANY_VAL_ACCESSOR( Object,	data.Object	= new ObjectHolder<RawType>(std::forward<T>(val)),
				if (ObjectHolder<T>* objHolder = dynamic_cast<ObjectHolder<T>*>(data.Object))
					return &objHolder->Object;
				return NULL;
		);

		ANY_VAL_ACCESSOR( SerializableObject,	data.Object	= new ObjectHolder<ISerializablePtr>(std::forward<T>(val)),
				if (ObjectHolder<ISerializablePtr>* objHolder = dynamic_cast<ObjectHolder<ISerializablePtr>*>(data.Object))
					return &objHolder->Object;
				return NULL;
		);

#undef ANY_VAL_ACCESSOR

	}}

	class any
	{
		template < typename ValueType >
		friend const ValueType* any_cast(const any* operand);

		template < typename ValueType >
		friend ValueType* any_cast(any* operand);

		template < typename T >
		friend const T& any_cast(const any& operand);

		template < typename T >
		friend T& any_cast(any& operand);

		typedef Detail::any::AnyType		Type;
		typedef Detail::any::IObjectHolder	IObjectHolder;
		typedef Detail::any::DataType		DataType;

	private:
		Type		_type;
		DataType	_data;

	public:
		any() : _type(Type::Empty)
		{ }

		any(const any& other) : _type(Type::Empty)
		{ Copy(other); }

		any(any&& other) : _type(Type::Empty)
		{ Move(std::move(other)); }

		template < typename T >
		any(T&& val, typename EnableIf<!IsSame<any, typename Decay<T>::ValueT>::Value, int>::ValueT = 0) : _type(Type::Empty)
		{ Init<T>(std::forward<T>(val)); }

		~any()
		{ Destroy(); }

		any& operator = (const any& other)
		{
			Destroy();
			Copy(other);
			return *this;
		}

		any& operator = (any&& other)
		{
			Destroy();
			Move(std::move(other));
			return *this;
		}

		template < typename T >
		typename EnableIf<!IsSame<any, typename Decay<T>::ValueT>::Value, any>::ValueT& operator = (T&& val)
		{
			Destroy();
			Init<T>(std::forward<T>(val));
			return *this;
		}

		bool empty() const { return _type == Type::Empty; }

		bool IsSerializable() const;

		std::string ToString() const;

		void Serialize(ObjectOStream& ar) const;
		void Deserialize(ObjectIStream& ar);

	private:
		template < typename T >
		void Init(T&& val)
		{
			typedef typename Decay<T>::ValueT RawType;

			_type = Detail::any::CppTypeToAnyUnionType<RawType>::Value;
			Detail::any::AnyValAccessor<Detail::any::CppTypeToAnyUnionType<RawType>::Value>::Set(_data, std::forward<T>(val));
		}

		template < typename T >
		T* Get() { return Detail::any::AnyValAccessor<Detail::any::CppTypeToAnyUnionType<T>::Value>::template Get<T>(_type, _data); }

		template < typename T >
		const T* Get() const { return Detail::any::AnyValAccessor<Detail::any::CppTypeToAnyUnionType<T>::Value>::template Get<T>(_type, _data); }

		void Copy(const any& other);
		void Move(any&& other);
		void Destroy();
	};


	class bad_any_cast : public std::bad_cast
	{
	private:
		std::string		_message;
	public:
		bad_any_cast(const std::string& from, const std::string& to) : _message(StringBuilder() % "Bad 'any' cast from " % from % " to " % to % "!") { }
		~bad_any_cast() noexcept override { }

		const char* what() const noexcept override { return _message.c_str(); }
	};


	template < typename ValueType >
	const ValueType* any_cast(const any* operand)
	{ return operand ? operand->template Get<ValueType>() : NULL; }

	template < typename ValueType >
	ValueType* any_cast(any* operand)
	{ return operand ? operand->template Get<ValueType>() : NULL; }

	template < typename T >
	T& any_cast(any& operand)
	{
		T* ptr = operand.template Get<T>();
		STINGRAYKIT_CHECK(ptr, bad_any_cast(operand._type.ToString(), Demangle(typeid(T).name())));
		return *ptr;
	}

	template < typename T >
	const T& any_cast(const any& operand)
	{
		const T* ptr = operand.template Get<T>();
		STINGRAYKIT_CHECK(ptr, bad_any_cast(operand._type.ToString(), Demangle(typeid(T).name())));
		return *ptr;
	}

}

#endif
