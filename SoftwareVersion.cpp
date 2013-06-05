#include <stingray/toolkit/SoftwareVersion.h>

#include <stingray/settings/Serialization.h>


namespace stingray
{

	void SoftwareVersion::Serialize(ObjectOStream& ar) const { ar.Serialize("major", _major).Serialize("minor", _minor).Serialize("build", _build); }
	void SoftwareVersion::Deserialize(ObjectIStream& ar) { ar.Deserialize("major", _major).Deserialize("minor", _minor).Deserialize("build", _build); }

}
