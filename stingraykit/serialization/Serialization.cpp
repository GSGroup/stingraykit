#include <stingraykit/serialization/Serialization.h>
#include <stingraykit/serialization/SettingsValue.h>
#include <queue>

namespace stingray
{

	SettingsValue* ObjectIStream::Get(const std::string &name)
	{
		SettingsValueMap &map = _root->get<SettingsValueMap>();
		SettingsValueMap::iterator i = map.find(name);
		return (i != map.end())? i->second.get(): NULL;
	}

	void ObjectIStream::for_each(const function<void (SettingsValue&)> &func)
	{
		SettingsValueList &list = _root->get<SettingsValueList>();
		for(SettingsValueList::iterator i = list.begin(); i != list.end(); ++i)
		{
			func(**i);
		}
	}

	void ObjectIStream::for_each_kv(const function<void (const std::string &, SettingsValue&)> &func)
	{
		SettingsValueMap &map = _root->get<SettingsValueMap>();
		for(SettingsValueMap::iterator i = map.begin(); i != map.end(); ++i)
		{
			func(i->first, *i->second);
		}
	}

	s64 ObjectIStream::GetInt() { return _root->get<s64>(); }

	bool ObjectIStream::is_null() const
	{ return _root->contains<EmptyType>(); }

	bool ObjectIStream::is_array() const
	{ return _root->contains<SettingsValueList>(); }

	void ObjectIStream::deserialize(std::string &str)
	{ str = _root->get<std::string>(); }

	void ObjectIStream::deserialize(bool &value)
	{ value = _root->get<bool>(); }

	void ObjectIStream::deserialize(double &value)
	{ value = _root->get<double>(); }

	void ObjectIStream::deserialize(std::vector<u8> & data)
	{
		if (_root->contains<std::vector<u8> >())
			data = _root->get<std::vector<u8> >();
		else if (_root->contains<std::string>())
			UnpackBinaryEncoding(data, _root->get<std::string>());
		else
			STINGRAYKIT_THROW("invalid type for binary data value");
	}

}
