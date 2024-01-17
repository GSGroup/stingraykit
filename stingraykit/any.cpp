// Copyright (c) 2011 - 2024, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/any.h>

#include <stingraykit/serialization/Serialization.h>
#include <stingraykit/assert.h>

namespace stingray
{

	namespace Detail {
	namespace any
	{
		void ObjectHolder<ISerializablePtr>::Serialize(ObjectOStream& ar) const
		{ ar.Serialize("obj", Object); }

		void ObjectHolder<ISerializablePtr>::Deserialize(ObjectIStream& ar)
		{ ar.Deserialize("obj", Object); }
	}}


	void any::Copy(const any& other)
	{
		STINGRAYKIT_ASSERT(_type == Type::Empty);

		switch (other._type)
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
			_data = other._data;
			break;
		case Type::String:
			_data.String.Ctor(other._data.String.Ref());
			break;
		case Type::Object:
		case Type::SerializableObject:
			_data.Object = other._data.Object->Clone();
			break;
		default:
			STINGRAYKIT_THROW(ArgumentException("type", other._type));
		}

		_type = other._type;
	}


	void any::Move(any&& other)
	{
		STINGRAYKIT_ASSERT(_type == Type::Empty);

		switch (other._type)
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
			_data = other._data;
			break;
		case Type::String:
			_data.String.Ctor(std::move(other._data.String.Ref()));
			break;
		case Type::Object:
		case Type::SerializableObject:
			_data.Object = other._data.Object;
			break;
		default:
			STINGRAYKIT_THROW(ArgumentException("type", other._type));
		}

		std::swap(_type, other._type);
	}


	void any::Destroy()
	{
		switch (_type)
		{
		case Type::String:
			_data.String.Dtor();
			break;
		case Type::Object:
		case Type::SerializableObject:
			delete _data.Object;
			break;
		default:
			break;
		}

		_type = Type::Empty;
	}


	bool any::IsSerializable() const
	{ return (_type != Type::Object && _type != Type::SerializableObject) || _data.Object->IsSerializable(); }


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

		STINGRAYKIT_THROW(StringBuilder() % "Unknown type: " % _type);
	}

#undef STRING


#define SERIALIZE(NAME) case Type::NAME: ar.Serialize("val", _data.NAME); return

	void any::Serialize(ObjectOStream& ar) const
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
				STINGRAYKIT_CHECK(_data.Object->IsSerializable(),
						StringBuilder() % "'any' object (" % Demangle(typeid(*_data.Object).name()) % ") is not a serializable one!");

				ar.Serialize(".class", _data.Object->GetClassName());
				_data.Object->Serialize(ar);
			}
			return;
		}

		STINGRAYKIT_THROW(StringBuilder() % "Unknown type: " % _type); //you could see warning about unhandled type if leave it here
	}

#undef SERIALIZE


#define DESERIALIZE(NAME) case Type::NAME: ar.Deserialize("val", _data.NAME); break

	void any::Deserialize(ObjectIStream& ar)
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

				unique_ptr<IObjectHolder> object(Factory::Instance().Create<IObjectHolder>(classname));
				object->Deserialize(ar);
				_data.Object = object.release();
			}
			break;
		default:
			STINGRAYKIT_THROW(StringBuilder() % "Unknown type: " % _type);
		}

		_type = type;
	}

#undef DESERIALIZE

}
