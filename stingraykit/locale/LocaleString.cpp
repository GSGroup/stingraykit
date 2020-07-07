// Copyright (c) 2011 - 2019, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/locale/LocaleString.h>

#include <stingraykit/locale/StringCodec.h>
#include <stingraykit/serialization/Serialization.h>

namespace stingray
{

	LocaleString::LocaleString(): TextEncoding(Encoding::ISO_8859_1), Text()
	{ }


	LocaleString::LocaleString(Encoding encoding, const std::string& text): TextEncoding(encoding), Text(text)
	{ }


	void LocaleString::Serialize(ObjectOStream& ar) const
	{
		ar.Serialize("encoding", TextEncoding);
		ar.Serialize("text", Text);
	}


	void LocaleString::Deserialize(ObjectIStream& ar)
	{
		ar.Deserialize("encoding", TextEncoding);
		ar.Deserialize("text", Text);
	}


	bool LocaleString::operator < (const LocaleString& other) const
	{ return Compare(other) < 0; }


	bool LocaleString::operator == (const LocaleString& other) const
	{
		if (TextEncoding == other.TextEncoding) //encoding matched
			return Text == other.Text;

		if (Text.size() != other.Text.size())
			return false;

		return Compare(other) == 0;
	}


	std::string LocaleString::ToString() const
	{ return StringCodec::ToUtf8(*this); }


	LocaleString LocaleString::FromUtf8(const std::string& str)
	{ return StringCodec::FromUtf8(str); }


	int LocaleString::Compare(const LocaleString& other) const
	{ return StringCodec::Compare(*this, other); }

}


