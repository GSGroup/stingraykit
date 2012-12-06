#ifndef STINGRAY_TOOLKIT_ANY_H
#define STINGRAY_TOOLKIT_ANY_H


#include <typeinfo>

#include <stingray/toolkit/StringUtils.h>
#include <stingray/toolkit/exception.h>


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

		struct IObjectHolder
		{
			virtual ~IObjectHolder() { }

			virtual IObjectHolder* Clone() const = 0;
			virtual std::string ToString() const = 0;
		};

		template < typename T, bool StringRepresentable = IsStringRepresentable<T>::Value >
		struct ObjectHolder : public virtual IObjectHolder
		{
			T	Object;
			ObjectHolder(const T& object) : Object(object) { }
			virtual IObjectHolder* Clone() const { return new ObjectHolder<T>(Object); }
			virtual std::string ToString() const { return "<not a string-representable type>"; }
		};

		template < typename T >
		struct ObjectHolder<T, true> : public virtual IObjectHolder
		{
			T	Object;
			ObjectHolder(const T& object) : Object(object) { }
			virtual IObjectHolder* Clone() const { return new ObjectHolder<T>(Object); }
			virtual std::string ToString() const { return stingray::ToString(Object); }
		};

		union DataType
		{
			int				Int;
			u8				U8;
			s8				S8;
			u16				U16;
			s16				S16;
			u32				U32;
			s32				S32;
			float			Float;
			double			Double;
			std::string*	String;
			IObjectHolder*	Object;
		};

		template < typename T >
		struct CppTypeToAnyUnionType
		{ static const AnyType::Enum Value = AnyType::Object; };

#define CPPTYPE_TO_ANYUNIONTYPE(Type_, EnumVal_) template < > struct CppTypeToAnyUnionType<Type_> { static const AnyType::Enum Value = AnyType::EnumVal_; }
		//CPPTYPE_TO_ANYUNIONTYPE( int,			Int );
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
			template < typename T > static const T& Get(AnyType type, const DataType& data) { TOOLKIT_CHECK(type == CppTypeToAnyUnionType<T>::Value, "Invalid 'any' type!"); Get_; } \
			template < typename T > static T& Get(AnyType type, DataType& data) { TOOLKIT_CHECK(type == CppTypeToAnyUnionType<T>::Value, "Invalid 'any' type!"); Get_; } \
		}

		ANY_VAL_ACCESSOR( Empty, 	/*empty*/,								TOOLKIT_THROW("'any' is empty!") );
		ANY_VAL_ACCESSOR( Int,		data.Int	= val,						return data.Int );
		ANY_VAL_ACCESSOR( U8,		data.U8		= val,						return data.U8 );
		ANY_VAL_ACCESSOR( S8,		data.S8		= val,						return data.S8 );
		ANY_VAL_ACCESSOR( U16,		data.U16	= val,						return data.U16 );
		ANY_VAL_ACCESSOR( S16,		data.S16	= val,						return data.S16 );
		ANY_VAL_ACCESSOR( U32,		data.U32	= val,						return data.U32 );
		ANY_VAL_ACCESSOR( S32,		data.S32	= val,						return data.S32 );
		ANY_VAL_ACCESSOR( Float,	data.Float	= val,						return data.Float );
		ANY_VAL_ACCESSOR( Double,	data.Double	= val,						return data.Double );
		ANY_VAL_ACCESSOR( String,	data.String	= new std::string(val),		return *data.String );
		ANY_VAL_ACCESSOR( Object,	data.Object	= new ObjectHolder<T>(val),	ObjectHolder<T>* obj_holder = dynamic_cast<ObjectHolder<T>*>(data.Object); if (obj_holder) return obj_holder->Object; TOOLKIT_THROW("Invalid 'any' type!") );
#undef ANY_VAL_ACCESSOR

	}}

	class any
	{
		template<typename ValueType> friend const ValueType * any_cast(const any * operand);
		template<typename ValueType> friend ValueType * any_cast(any * operand);

		typedef Detail::any::AnyType		Type;
		typedef Detail::any::IObjectHolder	IObjectHolder;
		typedef Detail::any::DataType		DataType;

	private:
		Type		_type;
		DataType	_data;

	public:
		any() : _type(Type::Empty) { }
		any(const any &other) { Copy(other._type, other._data); }

		template < typename T >
		any(const T & val) { Init<T>(val); }

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

		std::string ToString() const;

	private:
		template < typename T >
		void Init(const T& val)
		{
			_type = Detail::any::CppTypeToAnyUnionType<T>::Value;
			Detail::any::AnyValAccessor<Detail::any::CppTypeToAnyUnionType<T>::Value>::Set(_data, val);
		}

		template < typename T >
		T& Get() { return Detail::any::AnyValAccessor<Detail::any::CppTypeToAnyUnionType<T>::Value>::Get(_type, _data); }

		template < typename T >
		const T& Get() const { return Detail::any::AnyValAccessor<Detail::any::CppTypeToAnyUnionType<T>::Value>::Get(_type, _data); }

		void Copy(Type type, DataType data);
		void Destroy();
	};

	template < typename ValueType >
	const ValueType * any_cast(const any * operand)
	{ return &TOOLKIT_REQUIRE_NOT_NULL(operand)->template Get<ValueType>(); }

	template < typename ValueType >
	ValueType * any_cast(any * operand)
	{ return &TOOLKIT_REQUIRE_NOT_NULL(operand)->template Get<ValueType>(); }

	template < typename T >
	T any_cast(any & operand)
	{ return any_cast<T>(&operand); }

	template < typename T >
	T any_cast(const any & operand)
	{ return any_cast<T>(&operand); }


}


#endif
