#ifndef STINGRAY_TOOLKIT_ANY_H
#define STINGRAY_TOOLKIT_ANY_H


#include <iostream>
#include <typeinfo>

#include <stingray/settings/IsSerializable.h>
#include <stingray/settings/Serialization.h>
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
				Int,
				Bool,
				U8,
				S8,
				U16,
				S16,
				U32,
				S32,
				Float,
				Double,
				String,
				Object
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

		template < typename T >
		struct ObjectToString<T, true>
		{ static std::string ToString(const T& obj) { return stingray::ToString(obj); } };


		TOOLKIT_DECLARE_METHOD_CHECK(GetStaticFactoryObjectCreator);


		template < typename T >
		struct ObjectHolder;

		template
		<
			typename T,
			bool IsSerializable_ = IsSerializable<T>::Value && HasMethod_GetStaticFactoryObjectCreator<ObjectHolder<T> >::Value
		>
		struct ObjectSerializer
		{
			static void Serialize(ObjectOStream & ar, const T& obj) { TOOLKIT_THROW(NotSupportedException()); }
			static void Deserialize(ObjectIStream & ar, T& obj) { TOOLKIT_THROW(NotSupportedException()); }
			static bool IsSerializable() { return false; }
		};

		template < typename T >
		struct ObjectSerializer<T, true>
		{
			static void Serialize(ObjectOStream & ar, const T& obj)	{ ar.Serialize("obj", obj); }
			static void Deserialize(ObjectIStream & ar, T& obj)		{ ar.Deserialize("obj", obj); }
			static bool IsSerializable() { return true; }
		};


		template < typename T >
		struct ObjectHolderBase : public virtual IObjectHolder
		{
			T	Object;

			ObjectHolderBase() : Object()						{ }
			ObjectHolderBase(const T& object) : Object(object)	{ }

			virtual IObjectHolder* Clone() const				{ return new ObjectHolder<T>(Object); }
			virtual std::string ToString() const				{ return ObjectToString<T>::ToString(Object); }
			virtual void Serialize(ObjectOStream & ar) const	{ ObjectSerializer<T>::Serialize(ar, Object); }
			virtual void Deserialize(ObjectIStream & ar)		{ ObjectSerializer<T>::Deserialize(ar, Object); }
			virtual bool IsSerializable() const					{ return ObjectSerializer<T>::IsSerializable(); }
		};

		template < typename T >
		struct ObjectHolder : public ObjectHolderBase<T>
		{
			ObjectHolder(const T& object) : ObjectHolderBase<T>(object) { }

			virtual const IFactoryObjectCreator& GetFactoryObjectCreator() const { TOOLKIT_THROW(NotImplementedException()); }
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
				TOOLKIT_REGISTER_CLASS(ObjectHolder<__VA_ARGS__>); \
			}; \
		}}}

		union DataType
		{
			int						Int;
			bool					Bool;
			u8						U8;
			s8						S8;
			u16						U16;
			s16						S16;
			u32						U32;
			s32						S32;
			float					Float;
			double					Double;
			StorageFor<std::string>	String;
			IObjectHolder*			Object;
		};

		template < typename T >
		struct CppTypeToAnyUnionType
		{ static const AnyType::Enum Value = AnyType::Object; };

#define CPPTYPE_TO_ANYUNIONTYPE(Type_, EnumVal_) template < > struct CppTypeToAnyUnionType<Type_> { static const AnyType::Enum Value = AnyType::EnumVal_; }
		//CPPTYPE_TO_ANYUNIONTYPE( int,			Int );
		CPPTYPE_TO_ANYUNIONTYPE( bool,			Bool );
		CPPTYPE_TO_ANYUNIONTYPE( u8,			U8 );
		CPPTYPE_TO_ANYUNIONTYPE( s8,			S8 );
		CPPTYPE_TO_ANYUNIONTYPE( u16,			U16 );
		CPPTYPE_TO_ANYUNIONTYPE( s16,			S16 );
		CPPTYPE_TO_ANYUNIONTYPE( u32,			U32 );
		CPPTYPE_TO_ANYUNIONTYPE( s32,			S32 );
		CPPTYPE_TO_ANYUNIONTYPE( float,			Float );
		CPPTYPE_TO_ANYUNIONTYPE( double,		Double );
		CPPTYPE_TO_ANYUNIONTYPE( std::string,	String );
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
		ANY_VAL_ACCESSOR( Int,		data.Int	= val,						return &data.Int );
		ANY_VAL_ACCESSOR( Bool,		data.Bool	= val,						return &data.Bool );
		ANY_VAL_ACCESSOR( U8,		data.U8		= val,						return &data.U8 );
		ANY_VAL_ACCESSOR( S8,		data.S8		= val,						return &data.S8 );
		ANY_VAL_ACCESSOR( U16,		data.U16	= val,						return &data.U16 );
		ANY_VAL_ACCESSOR( S16,		data.S16	= val,						return &data.S16 );
		ANY_VAL_ACCESSOR( U32,		data.U32	= val,						return &data.U32 );
		ANY_VAL_ACCESSOR( S32,		data.S32	= val,						return &data.S32 );
		ANY_VAL_ACCESSOR( Float,	data.Float	= val,						return &data.Float );
		ANY_VAL_ACCESSOR( Double,	data.Double	= val,						return &data.Double );
		ANY_VAL_ACCESSOR( String,	data.String.Ctor(val); std::cout << data.String.Ref(),					return &data.String.Ref() );
		ANY_VAL_ACCESSOR( Object,	data.Object	= new ObjectHolder<T>(val),	ObjectHolder<T>* obj_holder = dynamic_cast<ObjectHolder<T>*>(data.Object); if (obj_holder) return &obj_holder->Object; return NULL; );
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
		bad_any_cast() : _message("Bad 'any' cast!") { }
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
			TOOLKIT_THROW(bad_any_cast());
		return *ptr;
	}

	template < typename T >
	const T& any_cast(const any & operand)
	{
		const T* ptr = operand.template Get<T>();
		if (!ptr)
			TOOLKIT_THROW(bad_any_cast());
		return *ptr;
	}


}


#endif
