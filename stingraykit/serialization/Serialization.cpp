// Copyright (c) 2011 - 2019, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/serialization/Serialization.h>
#include <stingraykit/serialization/SettingsValue.h>

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
	{ value = _root->contains<FloatString>() ? _root->get<FloatString>().ToDouble() : GetInt(); }

	void ObjectIStream::deserialize(FloatString &value)
	{ value = _root->contains<FloatString>() ? _root->get<FloatString>() : FloatString(GetInt()); }

	void ObjectIStream::deserialize(std::vector<u8> & data)
	{
		if (_root->contains<ByteArray>())
		{
			ConstByteData storage = _root->get<ByteArray>();
			data.assign(storage.begin(), storage.end());
		}
		else if (_root->contains<std::string>())
			UnpackBinaryEncoding(data, _root->get<std::string>());
		else
			STINGRAYKIT_THROW("invalid type for binary data value: " + TypeInfo(_root->type()).ToString());
	}


	static inline u8 xdigit(char c) {
		if (c >= '0' && c <= '9')
			return c - '0';
		c |= 0x20; //lowercasing
		if (c >= 'a' && c <= 'f')
			return c - 'a' + 10;
		STINGRAYKIT_THROW(std::runtime_error(std::string("invalid hex digit") + c));
	}

	void ObjectIStream::UnpackBinaryEncoding(std::vector<u8> & data, const std::string & str)
	{
		if (str.size() < 4 || str.compare(0, 4, "hex=") != 0)
			STINGRAYKIT_THROW(std::runtime_error("invalid binary encoding"));

		data.resize((str.size() - 4) / 2);
		std::vector<u8>::iterator dst = data.begin();
		for(std::string::const_iterator i = str.begin() + 4; i != str.end(); )
		{
			u8 h = xdigit(*i++) << 4;
			if (i == str.end())
				STINGRAYKIT_THROW(std::runtime_error("half-byte in hex dump"));
			*dst++ = xdigit(*i++) | h;
		}
	}

}
