#include <stingray/toolkit/UUID.h>

#include <stdlib.h>
#include <algorithm>

#include <stingray/toolkit/serialization/Serialization.h>


namespace stingray
{

	namespace
	{
		const std::string UuidFormat = "XXXX-XX-XX-XX-XXXXXX";
	}


	bool UUID::operator<(const UUID& other) const
	{
		return std::lexicographical_compare(_data.begin(), _data.end(), other._data.begin(), other._data.end());
	}


	void UUID::SerializeAsValue(ObjectOStream & ar) const
	{
		std::vector<u8> v(_data.begin(), _data.end());
		TypeWriter<std::vector<u8>, ObjectOStream>::Serialize(ar, v);
	}


	void UUID::DeserializeAsValue(ObjectIStream & ar)
	{
		std::vector<u8> v;
		TypeWriter<std::vector<u8>, ObjectIStream>::Deserialize(ar, v);
		STINGRAYKIT_CHECK(v.size() == _data.size(), "UUID size mismatch!");
		std::copy(v.begin(), v.end(), _data.begin());
	}


	void UUID::GenerateImpl(UUID& uuid)
	{
		for (DataType::iterator it = uuid._data.begin(); it != uuid._data.end(); ++it)
			*it = rand();
	}

	UUID UUID::FromString(const std::string& str)
	{
		STINGRAYKIT_CHECK(str.size() == static_cast<std::string::size_type>(2 * std::count(UuidFormat.begin(), UuidFormat.end(), 'X') + std::count(UuidFormat.begin(), UuidFormat.end(), '-')), "Invalid format");

		UUID result;

		DataType::iterator data_it = result._data.begin();
		for (std::string::const_iterator format_it = UuidFormat.begin(), str_it = str.begin(); format_it != UuidFormat.end(); ++format_it)
		{
			if (*format_it == 'X')
				*data_it++ = FromHex<u8>(std::string((str_it++).base(), 2 * sizeof(u8)));
			else
				STINGRAYKIT_CHECK(*str_it == '-', "Expected '-'");
			++str_it;
		}

		return result;
	}

	std::string UUID::ToString() const
	{
		std::string result;

		DataType::const_iterator data_it = _data.begin();
		for (std::string::const_iterator format_it = UuidFormat.begin(); format_it != UuidFormat.end(); ++format_it)
			if (*format_it == 'X')
				result += ToHex(*data_it++, 2 * sizeof(u8));
			else
				result += *format_it;

		return result;
	}

}
