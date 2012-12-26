#include <stingray/toolkit/any.h>


namespace stingray
{

	void any::Copy(Type type, const DataType& data)
	{
		switch (type)
		{
		case Type::Empty:
		case Type::Int:
		case Type::Bool:
		case Type::U8:
		case Type::S8:
		case Type::U16:
		case Type::S16:
		case Type::U32:
		case Type::S32:
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


	std::string any::ToString() const
	{
		switch (_type)
		{
		case Type::Empty:	return "<empty>";
		case Type::Int:		return stingray::ToString(_data.Int);
		case Type::Bool:	return stingray::ToString(_data.Bool);
		case Type::U8:		return stingray::ToString(_data.U8);
		case Type::S8:		return stingray::ToString(_data.S8);
		case Type::U16:		return stingray::ToString(_data.U16);
		case Type::S16:		return stingray::ToString(_data.S16);
		case Type::U32:		return stingray::ToString(_data.U32);
		case Type::S32:		return stingray::ToString(_data.S32);
		case Type::Float:	return stingray::ToString(_data.Float);
		case Type::Double:	return stingray::ToString(_data.Double);
		case Type::String:	return _data.String.Ref();
		case Type::Object:	return _data.Object->ToString();
		default:			TOOLKIT_THROW("Unknown type: " + _type.ToString());
		}
		return "";
	}


	void any::Serialize(ObjectOStream & ar) const
	{
		ar.Serialize("type", _type);
		switch (_type)
		{
		case Type::Empty:	return;
		case Type::Int:		ar.Serialize("val", _data.Int);				break;
		case Type::Bool:	ar.Serialize("val", _data.Bool);			break;
		case Type::U8:		ar.Serialize("val", _data.U8);				break;
		case Type::S8:		ar.Serialize("val", _data.S8);				break;
		case Type::U16:		ar.Serialize("val", _data.U16);				break;
		case Type::S16:		ar.Serialize("val", _data.S16);				break;
		case Type::U32:		ar.Serialize("val", _data.U32);				break;
		case Type::S32:		ar.Serialize("val", _data.S32);				break;
		case Type::Float:	ar.Serialize("val", _data.Float);			break;
		case Type::Double:	ar.Serialize("val", _data.Double);			break;
		case Type::String:	ar.Serialize("val", _data.String.Ref());	break;
		case Type::Object:
			{
				TOOLKIT_CHECK(_data.Object->IsSerializable(), "'any' object is not a serializable one!");
				const IFactoryObjectCreator& creator = _data.Object->GetFactoryObjectCreator();
				std::string classname(creator.Name());
				ar.Serialize(".class", classname);
				_data.Object->Serialize(ar);
			}
			break;
		default:			TOOLKIT_THROW("Unknown type: " + _type.ToString());
		}
	}


	void any::Deserialize(ObjectIStream & ar)
	{
		Destroy();

		Type type = Type::Empty;
		ar.Deserialize("type", type);
		switch (type)
		{
		case Type::Empty:	break;
		case Type::Int:		ar.Deserialize("val", _data.Int);		break;
		case Type::Bool:	ar.Deserialize("val", _data.Bool);		break;
		case Type::U8:		ar.Deserialize("val", _data.U8);		break;
		case Type::S8:		ar.Deserialize("val", _data.S8);		break;
		case Type::U16:		ar.Deserialize("val", _data.U16);		break;
		case Type::S16:		ar.Deserialize("val", _data.S16);		break;
		case Type::U32:		ar.Deserialize("val", _data.U32);		break;
		case Type::S32:		ar.Deserialize("val", _data.S32);		break;
		case Type::Float:	ar.Deserialize("val", _data.Float);		break;
		case Type::Double:	ar.Deserialize("val", _data.Double);	break;
		case Type::String:
			{
				std::string s;
				ar.Deserialize("val", s);
				Init(s);
			}
			break;
		case Type::Object:
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


}
