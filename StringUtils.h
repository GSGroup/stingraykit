#ifndef __GS_STINGRAY_TOOLKIT_STRINGUTILS_H__
#define __GS_STINGRAY_TOOLKIT_STRINGUTILS_H__

#include <sstream>
#include <vector>

#include <stingray/toolkit/Dummy.h>
#include <stingray/toolkit/IStringRepresentable.h>
#include <stingray/toolkit/NestedTypeCheck.h>
#include <stingray/toolkit/Types.h>
#include <stingray/toolkit/shared_ptr.h>
#include <stingray/toolkit/Tuple.h>
#include <stingray/toolkit/IEnumerable.h>


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

	namespace Detail
	{
		TOOLKIT_DECLARE_METHOD_CHECK(begin);
		TOOLKIT_DECLARE_METHOD_CHECK(end);
		TOOLKIT_DECLARE_METHOD_CHECK(ToString);

		template< typename ObjectType, bool HasBeginEnd = HasMethod_begin<ObjectType>::Value && HasMethod_end<ObjectType>::Value>
		struct TypeToStringSerializer;

		template< typename ObjectType>
		struct TypeToStringSerializer<ObjectType, true>
		{
			static std::string ToStringImpl(const ObjectType& object)
			{
				typename ObjectType::const_iterator it = object.begin(), iend = object.end();
				std::string result = "[";
				if (it != iend)
					result += ToString(*it++);
				while (it != iend)
					result += ", " + ToString(*it++);
				result += "]";
				return result;
			}
		};

		template<typename T>
		struct TypeToStringSerializer<IEnumerable<T>, false>
		{
			static std::string ToStringImpl(const IEnumerable<T>& enumerable)
			{
				shared_ptr<IEnumerator<T> > e = TOOLKIT_REQUIRE_NOT_NULL(enumerable.GetEnumerator());
				std::string result = "[";
				if (e->Valid())
				{
					result += ToString(e->Get());
					e->Next();
				}
				while (e->Valid())
				{
					result += ", " + ToString(e->Get());
					e->Next();
				}
				result += "]";
				return result;
			}
		};

		template< typename ObjectType>
		struct TypeToStringSerializer<ObjectType, false>
		{
			static std::string ToStringImpl(const ObjectType& val)
			{
				std::ostringstream s;
				s << val;
				return s.str();
			}
		};

		template<>
		struct TypeToStringSerializer<u8, false>
		{
			static std::string ToStringImpl(u8 val)
			{
				std::ostringstream s;
				s << (int)val;
				return s.str();
			}
		};

		template<>
		struct TypeToStringSerializer<std::string, true>
		{
			static std::string ToStringImpl(const std::string& str)
			{ return str; }
		};

		template<>
		struct TypeToStringSerializer<const char*, false>
		{
			static std::string ToStringImpl(const char* str)
			{ return str; }
		};

		template<typename T>
		struct TypeToStringSerializer<shared_ptr<T>, false>
		{
			static std::string ToStringImpl(const shared_ptr<T>& ptr)
			{ return ptr ? ToString(*ptr) : "null"; }
		};

		template< typename ObjectType, bool HasToStringMethod = HasMethod_ToString<ObjectType>::Value >
		struct ToStringHelper;

		template< typename ObjectType >
		struct ToStringHelper<ObjectType, true>
		{
			static std::string ToString(const ObjectType& object)
			{ return object.ToString(); }
		};

		template< typename ObjectType >
		struct ToStringHelper<ObjectType, false>
		{
			static std::string ToString(const ObjectType& object)
			{ return TypeToStringSerializer<ObjectType>::ToStringImpl(object); }
		};
	}

	template < typename T >
	std::string ToString(const T& val)
	{ return Detail::ToStringHelper<T>::ToString(val); }

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

	inline void ReplaceAll(std::string& str, const std::string& replaceSeq, const std::string& replaceTo)
	{ ReplaceAll<char>(str, replaceSeq, replaceTo); }

	inline std::string ExtractPrefix(const std::string& str, size_t prefixLength)
	{ return str.substr(0, std::min(str.length(), prefixLength)); }

	inline std::string ExtractSuffix(const std::string& str, size_t suffixLength)
	{
		const size_t length = std::min(str.length(), suffixLength);
		return str.substr(str.length() - length, length);
	}

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
	{
		const size_t pos = str.find_last_not_of(ch);
		return pos == std::string::npos? str : str.substr(0, pos + 1);
	}

	inline std::string LeftStrip(const std::string& str, char ch = ' ')
	{
		const size_t pos = str.find_first_not_of(ch);
		return pos == std::string::npos? str : str.substr(pos);
	}

	inline std::string Strip(const std::string& str, char ch = ' ')
	{ return LeftStrip(RightStrip(str, ch), ch); }


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
			_stream << stingray::ToString(object);
			return *this;
		}

		operator StringType() const
		{ return _stream.str(); }

		std::string ToString() const
		{ return _stream.str(); }
	};

	typedef BasicStringBuilder<char>	StringBuilder;
	typedef BasicStringBuilder<wchar_t>	WideStringBuilder;


}

/*! \endcond */


#endif
