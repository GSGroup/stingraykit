#include <stingray/toolkit/any.h>

#include <stingray/settings/Serialization.h>

namespace stingray
{

	namespace Detail {
	namespace any
	{
		void ObjectHolder<ISerializablePtr>::Serialize(ObjectOStream & ar) const
		{ ar.Serialize("obj", Object); }

		void ObjectHolder<ISerializablePtr>::Deserialize(ObjectIStream & ar)
		{ ar.Deserialize("obj", Object); }
	}}


	void any::Copy(Type type, const DataType& data)
	{
		switch (type)
		{
		case Type::Empty:
		case Type::Bool:
		case Type::Char:
		case Type::UChar:
		case Type::Short:
		case Type::UShort:
		case Type::Int:
		case Type::UInt:
		case Type::Long:
		case Type::ULong:
		case Type::LongLong:
		case Type::ULongLong:
		case Type::Float:
		case Type::Double:
			_data = data;
			break;
		case Type::String:
			_data.String.Ctor(data.String.Ref());
			break;
		case Type::Object:
			_data.Object = data.Object->Clone();
			break;
		default:
			TOOLKIT_THROW(ArgumentException("type", type.ToString()));
		}
		_type = type;
	}


	void any::Destroy()
	{
		switch (_type)
		{
		case Type::String:	_data.String.Dtor();	break;
		case Type::Object:	delete _data.Object;	break;
		default:			break;
		}

		_type = Type::Empty;
	}


	bool any::IsSerializable() const
	{ return _type != Type::Object || _data.Object->IsSerializable(); }

#define STRING(NAME) case Type::NAME: return stingray::ToString(_data.NAME)

	std::string any::ToString() const
	{
		switch (_type)
		{
		case Type::Empty:	return "<empty>";
		STRING(Bool);
		STRING(Char);
		STRING(UChar);
		STRING(Short);
		STRING(UShort);
		STRING(Int);
		STRING(UInt);
		STRING(Long);
		STRING(ULong);
		STRING(LongLong);
		STRING(ULongLong);
		STRING(Float);
		STRING(Double);
		case Type::String:
			return _data.String.Ref();
		case Type::Object:
		case Type::SerializableObject:
			return _data.Object->ToString();
		}
		TOOLKIT_THROW("Unknown type: " + _type.ToString());
	}

#undef STRING

#define SERIALIZE(NAME) case Type::NAME: ar.Serialize("val", _data.NAME); return

	void any::Serialize(ObjectOStream & ar) const
	{
		ar.Serialize("type", _type);
		switch (_type)
		{
		case Type::Empty:	return;
		SERIALIZE(Bool);
		SERIALIZE(Char);
		SERIALIZE(UChar);
		SERIALIZE(Short);
		SERIALIZE(UShort);
		SERIALIZE(Int);
		SERIALIZE(UInt);
		SERIALIZE(Long);
		SERIALIZE(ULong);
		SERIALIZE(LongLong);
		SERIALIZE(ULongLong);
		SERIALIZE(Float);
		SERIALIZE(Double);
		case Type::String: ar.Serialize("val", _data.String.Ref()); return;
		case Type::Object:
		case Type::SerializableObject:
			{
				TOOLKIT_CHECK(_data.Object->IsSerializable(), "'any' object (" + Demangle(typeid(*_data.Object).name()) + ") is not a serializable one!");
				std::string classname(_data.Object->GetClassName());
				ar.Serialize(".class", classname);
				_data.Object->Serialize(ar);
			}
			return;
		}
		TOOLKIT_THROW("Unknown type: " + _type.ToString());//you could see warning about unhandled type if leave it here
	}
#undef SERIALIZE

#define DESERIALIZE(NAME) case Type::NAME: ar.Deserialize("val", _data.NAME); break

	void any::Deserialize(ObjectIStream & ar)
	{
		Destroy();

		Type type = Type::Empty;
		ar.Deserialize("type", type);
		switch (type)
		{
		case Type::Empty:	break;
		DESERIALIZE(Bool);
		DESERIALIZE(Char);
		DESERIALIZE(UChar);
		DESERIALIZE(Short);
		DESERIALIZE(UShort);
		DESERIALIZE(Int);
		DESERIALIZE(UInt);
		DESERIALIZE(Long);
		DESERIALIZE(ULong);
		DESERIALIZE(LongLong);
		DESERIALIZE(ULongLong);
		DESERIALIZE(Float);
		DESERIALIZE(Double);
		case Type::String:
			{
				std::string s;
				ar.Deserialize("val", s);
				Init(s);
			}
			break;
		case Type::Object:
		case Type::SerializableObject:
			{
				std::string classname;
				ar.Deserialize(".class", classname);
				_data.Object = Factory::Instance().Create<IObjectHolder>(classname);
				_data.Object->Deserialize(ar);
			}
			break;
		default:			TOOLKIT_THROW("Unknown type: " + _type.ToString());
		}
		_type = type;
	}

#undef DESERIALIZE

}
