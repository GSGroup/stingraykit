#ifndef __GS_STINGRAY_TOOLKIT_STRINGUTILS_H__
#define __GS_STINGRAY_TOOLKIT_STRINGUTILS_H__

#include <sstream>
#include <vector>

#include <stingray/toolkit/Dummy.h>
#include <stingray/toolkit/Types.h>
#include <stingray/toolkit/IStringRepresentable.h>


/*! \cond GS_INTERNAL */

namespace stingray
{

	template < typename T>
	T FromHex(const std::string &str)
	{
		size_t n = str.size();
		T r = T();
		for(size_t i = 0; i < n; ++i)
		{
			char c = str[i];
			if (c >= '0' && c <= '9')
				c -= '0';
			else
			{
				c &= ~ 0x20;
				if (c >= 'A' && c <= 'F')
					c = c - 'A' + 10;
				else
					throw std::runtime_error(std::string("invalid char '") + str[i] + "' in hex string");
			}
			r |= c << ((n - i - 1) * 4);
		}
		return r;
	}

	template < typename CharType, typename T >
	std::basic_string<CharType> ToString(const T& val, Dummy dummy = Dummy())
	{
		std::basic_stringstream<CharType> s;
		s << val;
		return s.str();
	}

	template < typename CharType >
	std::basic_string<CharType> ToString(u8 val, Dummy dummy = Dummy())
	{
		std::basic_stringstream<CharType> s;
		s << (int)val;
		return s.str();
	}

	template < typename T >
	T FromString(const std::string& str) // TODO: reimplement
	{
		std::stringstream s(str);
		T val = T();
		s >> val;
		if (!s.eof())
			throw std::runtime_error("FromString: Could not parse value!"); // =(

		return val;
	}

	template < typename T >
	std::string ToString(const T& val)
	{ return ToString<char, T>(val); }

	/////////////////////////////////////////////////////////////////


	template < typename CharType >
	void ReplaceAll(std::basic_string<CharType>& str,
					const std::basic_string<CharType>& replaceSeq,
					const std::basic_string<CharType>& replaceTo,
					Dummy dummy = Dummy())
	{
		typedef std::basic_string<CharType>	StrType;

		typename StrType::size_type i = str.find(replaceSeq);
		while (i != StrType::npos)
		{
			str.replace(i, replaceSeq.size(), replaceTo);
			i = str.find(replaceSeq, i + replaceTo.size());
		}
	}

	inline std::string ExtractPrefix(const std::string& str, size_t prefixLength)
	{ return str.substr(0, std::min(str.length(), prefixLength)); }

	inline std::string ExtractSuffix(const std::string& str, size_t suffixLength)
	{
		const size_t length = std::min(str.length(), suffixLength);
		return str.substr(str.length() - length, length);
	}

	inline void ReplaceAll(std::string& str, const std::string& replaceSeq, const std::string& replaceTo)
	{ ReplaceAll<char>(str, replaceSeq, replaceTo); }

	inline bool BeginsWith(const std::string& str, const std::string& prefix)
	{ return str.length() >= prefix.length() && ExtractPrefix(str, prefix.length()) == prefix; }

	inline bool EndsWith(const std::string& str, const std::string& suffix)
	{ return str.length() >= suffix.length() && ExtractSuffix(str, suffix.length()) == suffix; }

	inline void Split(const std::string& str, const std::string& delim, std::vector<std::string>& result)
	{
		size_t i = 0, j;
		while ((j = str.find(delim, i)) != std::string::npos)
		{
			result.push_back(str.substr(i, j - i));
			i = j + delim.length();
		}
		result.push_back(str.substr(i));
	}

	inline std::string RightStrip(const std::string& str, char ch = ' ')
	{ return str.substr(0, str.find_last_not_of(ch)); }

	inline std::string LeftStrip(const std::string& str, char ch = ' ')
	{ return str.substr(0, str.find_first_not_of(ch)); }

	inline std::string Strip(const std::string& str, char ch = ' ')
	{ return LeftStrip(RightStrip(str, ch), ch); }


	template <typename CharType >
	class BasicStringReader
	{
		typedef std::basic_istringstream<CharType>	StreamType;
		typedef std::basic_string<CharType>			StringType;

	private:
		StreamType	_stream;

	public:
		explicit BasicStringReader(const StringType& str)
			: _stream(str)
		{ }

		template<typename T>
		T Read()
		{
			T result = T();

			_stream >> result;
			if (_stream.fail())
				throw std::runtime_error("couldn't read value!");

			return result;
		}

		StringType ReadStringUntil(char delimeter)
		{
			StringType result;

			if (std::getline(_stream, result, delimeter).fail())
				throw std::runtime_error("couldn't read value!");

			return result;
		}
	};

	typedef BasicStringReader<char>		StringReader;
	typedef BasicStringReader<wchar_t>	WideStringReader;


	namespace Detail
	{

		template< typename StreamType, typename ObjectType, bool HasToStringMethod = HasMethod_ToString<ObjectType>::Value >
		struct TypeSerializer;

		template< typename StreamType, typename ObjectType >
		struct TypeSerializer<StreamType, ObjectType, true>
		{
			static void Serialize(StreamType& stream, const ObjectType& object)
			{ stream << object.ToString(); }
		};

		template< typename StreamType, typename ObjectType >
		struct TypeSerializer<StreamType, ObjectType, false>
		{
			static void Serialize(StreamType& stream, const ObjectType& object)
			{ stream << object; }
		};

	}


	template< typename CharType >
	class BasicStringBuilder
	{
		typedef std::basic_ostringstream<CharType>	StreamType;
		typedef std::basic_string<CharType>			StringType;

	private:
		StreamType	_stream;

	public:
		template<typename ObjectType>
		BasicStringBuilder& operator % (const ObjectType& object)
		{
			Detail::TypeSerializer<StreamType, ObjectType>::Serialize(_stream, object);
			return *this;
		}

		operator StringType() const
		{ return _stream.str(); }
	};

	typedef BasicStringBuilder<char>	StringBuilder;
	typedef BasicStringBuilder<wchar_t>	WideStringBuilder;


}

/*! \endcond */


#endif
