#include <stingray/toolkit/any.h>


namespace stingray
{

	void any::Copy(Type type, DataType data)
	{
		_type = type;
		switch (_type)
		{
		case Type::Empty:
		case Type::Int:
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
			_data.String = new std::string(*data.String);
			break;
		case Type::Object:
			_data.Object = data.Object->Clone();
			break;
		default:
			TOOLKIT_THROW(ArgumentException("type", type.ToString()));
		}

	}


	void any::Destroy()
	{
		switch (_type)
		{
		case Type::String:	delete _data.String;	break;
		case Type::Object:	delete _data.Object;	break;
		default:			break;
		}

		_type = Type::Empty;
	}


	std::string any::ToString() const
	{
		switch (_type)
		{
		case Type::Empty:	return "<empty>";							break;
		case Type::Int:		return stingray::ToString(_data.U8);		break;
		case Type::U8:		return stingray::ToString(_data.U8);		break;
		case Type::S8:		return stingray::ToString(_data.S8);		break;
		case Type::U16:		return stingray::ToString(_data.U16);		break;
		case Type::S16:		return stingray::ToString(_data.S16);		break;
		case Type::U32:		return stingray::ToString(_data.U32);		break;
		case Type::S32:		return stingray::ToString(_data.S32);		break;
		case Type::Float:	return stingray::ToString(_data.Float);		break;
		case Type::Double:	return stingray::ToString(_data.Double);	break;
		case Type::String:	return *_data.String;						break;
		case Type::Object:	return _data.Object->ToString();			break;
		default:			TOOLKIT_THROW("Unknown type: " + _type.ToString());
		}
		return "";
	}

}
