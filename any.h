#ifndef STINGRAY_TOOLKIT_ANY_H
#define STINGRAY_TOOLKIT_ANY_H


#include <iostream>
#include <typeinfo>

#include <stingray/settings/ISerializable.h>
#include <stingray/toolkit/Factory.h>
#include <stingray/toolkit/MetaProgramming.h>
#include <stingray/toolkit/StringUtils.h>
#include <stingray/toolkit/aligned_storage.h>
#include <stingray/toolkit/exception.h>
#include <stingray/toolkit/toolkit.h>


namespace stingray
{

	namespace Detail {
	namespace any
	{
		struct AnyType
		{
			TOOLKIT_ENUM_VALUES
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
			TOOLKIT_DECLARE_ENUM_CLASS(AnyType);
		};


		struct IObjectHolder : public virtual ISerializable, public virtual IFactoryObject
		{
			virtual ~IObjectHolder() { }

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

			ObjectHolderBase() : Object()						{ }
			ObjectHolderBase(const T& object) : Object(object)	{ }

			virtual IObjectHolder* Clone() const				{ return new ObjectHolder<T>(Object); }
			virtual std::string ToString() const				{ return ObjectToString<T>::ToString(Object); }
		};

		template < typename T >
		struct ObjectHolder : public ObjectHolderBase<T>
		{
			ObjectHolder(const T& object) : ObjectHolderBase<T>(object) { }

			virtual void Serialize(ObjectOStream & ar) const	{ TOOLKIT_THROW(NotSupportedException()); }
			virtual void Deserialize(ObjectIStream & ar)		{ TOOLKIT_THROW(NotSupportedException()); }
			virtual bool IsSerializable() const					{ return false; }

			virtual std::string GetClassName() const { TOOLKIT_THROW(NotImplementedException()); }
		};

		template < >
		struct ObjectHolder<ISerializablePtr> : public ObjectHolderBase<ISerializablePtr>
		{
			ObjectHolder() { }
			ObjectHolder(const ISerializablePtr& object) : ObjectHolderBase<ISerializablePtr>(object) { }
			virtual void Serialize(ObjectOStream & ar) const;
			virtual void Deserialize(ObjectIStream & ar);
			virtual bool IsSerializable() const					{ return true; }
			TOOLKIT_REGISTER_CLASS(ObjectHolder<ISerializablePtr>);
		};

#define TOOLKIT_DECLARE_SERIALIZABLE_ANY_OBJECTHOLDER(...) \
		namespace stingray { \
		namespace Detail { \
		namespace any { \
			template < > \
			struct ObjectHolder<__VA_ARGS__> : public ObjectHolderBase<__VA_ARGS__> \
			{ \
				ObjectHolder() { } \
				ObjectHolder(const __VA_ARGS__& object) : ObjectHolderBase<__VA_ARGS__>(object) { } \
				virtual void Serialize(ObjectOStream & ar) const	{ ar.Serialize("obj", Object); } \
				virtual void Deserialize(ObjectIStream & ar)		{ ar.Deserialize("obj", Object); } \
				virtual bool IsSerializable() const					{ return true; } \
				TOOLKIT_REGISTER_CLASS(ObjectHolder<__VA_ARGS__>); \
			}; \
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
		struct IsPtrToISerializable
		{ static const bool Value = false; };

		template < typename T >
		struct IsPtrToISerializable<T, true>
		{ static const bool Value = Inherits<typename GetSharedPtrParam<T>::ValueT, ISerializable>::Value; };

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
			template < typename T > static void Set(DataType& data, const T& val) { Set_; } \
			template < typename T > static const T* Get(AnyType type, const DataType& data) { if (type != CppTypeToAnyUnionType<T>::Value) return NULL; Get_; } \
			template < typename T > static T* Get(AnyType type, DataType& data) { if (type != CppTypeToAnyUnionType<T>::Value) return NULL; Get_; } \
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
		ANY_VAL_ACCESSOR( String,	data.String.Ctor(val),					return &data.String.Ref() );
		ANY_VAL_ACCESSOR( Object,	data.Object	= new ObjectHolder<T>(val),	ObjectHolder<T>* obj_holder = dynamic_cast<ObjectHolder<T>*>(data.Object); if (obj_holder) return &obj_holder->Object; return NULL; );
		ANY_VAL_ACCESSOR( SerializableObject,	data.Object	= new ObjectHolder<ISerializablePtr>(val),	ObjectHolder<ISerializablePtr>* obj_holder = dynamic_cast<ObjectHolder<ISerializablePtr>*>(data.Object); if (obj_holder) return dynamic_cast<T*>(obj_holder->Object.get()); return NULL; );
#undef ANY_VAL_ACCESSOR

	}}

	class any
	{
		template<typename ValueType> friend const ValueType * any_cast(const any * operand);
		template<typename ValueType> friend ValueType * any_cast(any * operand);
		template<typename T> friend const T& any_cast(const any& operand);
		template<typename T> friend T& any_cast(any& operand);

		typedef Detail::any::AnyType		Type;
		typedef Detail::any::IObjectHolder	IObjectHolder;
		typedef Detail::any::DataType		DataType;

	private:
		Type		_type;
		DataType	_data;

	public:
		any() : _type(Type::Empty) { }
		any(const any &other) : _type(Type::Empty) { Copy(other._type, other._data); }

		template < typename T >
		any(const T & val) : _type(Type::Empty) { Init<T>(val); }

		~any() { Destroy(); }

		any & operator=(const any &other) { Destroy(); Copy(other._type, other._data); return *this; }

		template < typename T >
		any & operator=(const T & val) { Destroy(); Init<T>(val); return *this; }

		any & swap(any & other)
		{
			std::swap(_type, other._type);
			std::swap(_data, other._data);
			return *this;
		}

		bool empty() const { return _type == Type::Empty; }
		//const std::type_info & type() const;

		bool IsSerializable() const;

		std::string ToString() const;

		void Serialize(ObjectOStream & ar) const;
		void Deserialize(ObjectIStream & ar);

	private:
		template < typename T >
		void Init(const T& val)
		{
			_type = Detail::any::CppTypeToAnyUnionType<T>::Value;
			Detail::any::AnyValAccessor<Detail::any::CppTypeToAnyUnionType<T>::Value>::Set(_data, val);
		}

		template < typename T >
		T* Get() { return Detail::any::AnyValAccessor<Detail::any::CppTypeToAnyUnionType<T>::Value>::template Get<T>(_type, _data); }

		template < typename T >
		const T* Get() const { return Detail::any::AnyValAccessor<Detail::any::CppTypeToAnyUnionType<T>::Value>::template Get<T>(_type, _data); }

		void Copy(Type type, const DataType& data);
		void Destroy();
	};


	class bad_any_cast : public std::bad_cast
	{
	private:
		std::string		_message;
	public:
		bad_any_cast(const std::string& from, const std::string& to) : _message("Bad 'any' cast from " + from + " to " + to + "!") { }
		virtual ~bad_any_cast() throw() { }

		virtual const char* what() const throw() { return _message.c_str(); }
	};

	template < typename ValueType >
	const ValueType * any_cast(const any * operand)
	{ return operand ? operand->template Get<ValueType>() : NULL; }

	template < typename ValueType >
	ValueType * any_cast(any * operand)
	{ return operand ? operand->template Get<ValueType>() : NULL; }

	template < typename T >
	T& any_cast(any & operand)
	{
		T* ptr = operand.template Get<T>();
		if (!ptr)
			TOOLKIT_THROW(bad_any_cast(operand._type.ToString(), Demangle(typeid(T).name())));
		return *ptr;
	}

	template < typename T >
	const T& any_cast(const any & operand)
	{
		const T* ptr = operand.template Get<T>();
		if (!ptr)
			TOOLKIT_THROW(bad_any_cast(operand._type.ToString(), Demangle(typeid(T).name())));
		return *ptr;
	}


}


#endif
