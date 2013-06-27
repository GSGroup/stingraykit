#include <stingray/toolkit/StringUtils.h>


namespace stingray
{

	std::string Utf8ToLower(const std::string& str)
	{
		std::string result;
		for (std::string::const_iterator it = str.begin(); it != str.end(); ++it)
		{
			if (*it >= 'A' && *it <= 'Z')
				result.push_back(*it + 'a' - 'A');
			else if ((u8)*it == 0xD0)
			{
				u16 c0 = (u8)*it & 0x1F;
				TOOLKIT_CHECK(++it != str.end(), "Malformed utf-8!");
				u16 c = (c0 << 6) | ((u8)*it & 0x3F);
				if (c >= 0x410 && c <= 0x42F)
					c += 0x20;
				result.push_back(0xD0 | (c >> 6));
				result.push_back((c & 0x3F) | 0x80);
			}
			else
				result.push_back(*it);
		}
		return result;
	}

	void PackUtf8(std::string &dst, uint32_t ucs, uint32_t replacement)
	{
		if (ucs < 0x80)
		{
			dst += (char)ucs;
		}
		else if (ucs < 0x800)
		{
			dst += (char) ((ucs >> 6) | 0xc0);
			dst += (char) ((ucs & 0x3f) | 0x80);
		}
		else if (ucs < 0x10000)
		{
			dst += (char)((ucs >> 12) | 0xe0);
			dst += (char)(((ucs & 0x0fc0) >> 6) | 0x80);
			dst += (char)((ucs & 0x003f) | 0x80);
		}
		else if (ucs < 0x110000) //actually 0x200000, but unicode still not reached this limit.
		{
			dst += (char)((ucs >> 18) | 0xf0);
			dst += (char)(((ucs & 0x03f000) >> 12) | 0x80);
			dst += (char)(((ucs & 0x000fc0) >> 6) | 0x80);
			dst += (char)( (ucs & 0x00003f) | 0x80);
		}
		else
			dst += replacement;
	}


}
