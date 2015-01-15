#ifndef STINGRAYKIT_UUID_H
#define STINGRAYKIT_UUID_H


#include <stingraykit/serialization/ISerializable.h>
#include <stingraykit/collection/array.h>


namespace stingray {


	struct UUID
	{
	private:
		typedef array<u8, 16> DataType;
		DataType _data;

	public:
		void SerializeAsValue(ObjectOStream & ar) const;
		void DeserializeAsValue(ObjectIStream & ar);

		bool operator< (const UUID& other) const;
		STINGRAYKIT_GENERATE_COMPARISON_OPERATORS_FROM_LESS(UUID);

		static UUID Generate()
		{
			UUID result;
			GenerateImpl(result);
			return result;
		}

		static UUID FromString(const std::string& str);

		std::string ToString() const;

	private:
		static void GenerateImpl(UUID& uuid);
	};



}


#endif

