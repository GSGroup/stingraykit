#ifndef STINGRAY_TOOLKIT_UUID_H
#define STINGRAY_TOOLKIT_UUID_H


#include <stingray/settings/ISerializable.h>
#include <stingray/toolkit/array.h>


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
		TOOLKIT_GENERATE_COMPARISON_OPERATORS_FROM_LESS(UUID);

		static UUID Generate()
		{
			UUID result;
			GenerateImpl(result);
			return result;
		}

		std::string ToString() const;

	private:
		static void GenerateImpl(UUID& uuid);
	};



}


#endif

