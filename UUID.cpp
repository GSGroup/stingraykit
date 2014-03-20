#include <stingray/toolkit/UUID.h>

#include <stdlib.h>
#include <algorithm>

#include <stingray/settings/Serialization.h>


namespace stingray
{

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
		TOOLKIT_CHECK(v.size() == _data.size(), "UUID size mismatch!");
		std::copy(v.begin(), v.end(), _data.begin());
	}


	void UUID::GenerateImpl(UUID& uuid)
	{
		for (DataType::iterator it = uuid._data.begin(); it != uuid._data.end(); ++it)
			*it = rand();
	}


	std::string UUID::ToString() const
	{
		const std::string UuidFormat = "XXXX-XX-XX-XX-XXXXXX";

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


