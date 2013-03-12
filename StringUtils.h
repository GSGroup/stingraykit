#ifndef STINGRAY_TOOLKIT_STRINGUTILS_H
#define STINGRAY_TOOLKIT_STRINGUTILS_H

#include <algorithm>
#include <sstream>
#include <string>

#include <stingray/toolkit/Dummy.h>
#include <stingray/toolkit/IEnumerable.h>
#include <stingray/toolkit/NestedTypeCheck.h>
#include <stingray/toolkit/Types.h>
#include <stingray/toolkit/exception.h>
#include <stingray/toolkit/optional.h>
#include <stingray/toolkit/shared_ptr.h>


/*! \cond GS_INTERNAL */
namespace std
{
	template < class Key, class T, class Compare, class Allocator > class map;
}

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

	template<typename T>
	std::string ToHex(T value, size_t width = 0, bool capital = false, bool add0xPrefix = false)
	{
		std::string r;
		r.reserve(std::max(sizeof(value) * 2, add0xPrefix? width + 2: width));

		do
		{
			char c = value & 0x0f;
			r.insert(0, 1, (char)(c > 9? c + (capital? 'A': 'a') - 10: c + '0'));
			value >>= 4;
		}
		while(value != 0);

		if (r.size() < width)
			r.insert(0, std::string(width - r.size(), '0'));

		if (add0xPrefix)
			r.insert(0, "0x");

		return r;
	}

	template < typename T >
	T FromString(const std::string& str) // TODO: reimplement
	{
		std::stringstream s(str);
		T val = T();
		s >> val;
		if (!s.eof())
			TOOLKIT_THROW(ArgumentException("str", str));

		return val;
	}

	template < typename T >
	std::string ToString(const T& val);

	namespace Detail
	{
		TOOLKIT_DECLARE_METHOD_CHECK(begin);
		TOOLKIT_DECLARE_METHOD_CHECK(end);
		TOOLKIT_DECLARE_METHOD_CHECK(ToString);

		struct TypeToStringObjectType
		{
			TOOLKIT_ENUM_VALUES(HasBeginEnd, HasToString, Enumerable, IsException, Other, ProxyObjToStdStream);
			TOOLKIT_DECLARE_ENUM_CLASS(TypeToStringObjectType);
		};

		template
			<
				typename ObjectType,
				TypeToStringObjectType::Enum ObjType =
					HasMethod_ToString<ObjectType>::Value ?
						TypeToStringObjectType::HasToString :
						(HasMethod_begin<ObjectType>::Value && HasMethod_end<ObjectType>::Value ?
							TypeToStringObjectType::HasBeginEnd :
							(IsEnumerable<ObjectType>::Value ?
								TypeToStringObjectType::Enumerable :
								(Inherits<ObjectType, std::exception>::Value ?
									TypeToStringObjectType::IsException :
									(
										SameType<u8, ObjectType>::Value
											|| SameType<const char*, ObjectType>::Value
											|| Is1ParamTemplate<shared_ptr, ObjectType>::Value
											|| Is1ParamTemplate<optional, ObjectType>::Value
											|| Is2ParamTemplate<std::pair, ObjectType>::Value ?
										TypeToStringObjectType::Other :
										TypeToStringObjectType::ProxyObjToStdStream
									)
								)
							)
						)
			>
		struct TypeToStringObjectTypeGetter
		{ static const TypeToStringObjectType::Enum Value = ObjType; };

		template < typename ObjectType, TypeToStringObjectType::Enum ObjType = TypeToStringObjectTypeGetter<ObjectType>::Value >
		struct TypeToStringSerializer;

		template< typename ObjectType>
		struct TypeToStringSerializer<ObjectType, TypeToStringObjectType::HasBeginEnd>
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

		template< typename KeyType, typename ValueType, typename CompareType, typename AllocatorType >
		struct TypeToStringSerializer<std::map<KeyType, ValueType, CompareType, AllocatorType>, TypeToStringObjectType::HasBeginEnd>
		{
			typedef std::map<KeyType, ValueType, CompareType, AllocatorType>	MapType;
			static std::string ToStringImpl(const MapType& object)
			{
				typename MapType::const_iterator it = object.begin(), iend = object.end();
				std::string result = "{ ";
				if (it != iend)
				{
					result += ToString(it->first) + ": " + ToString(it->second);
					++it;
				}
				for (; it != iend; ++it)
					result += ", " + ToString(it->first) + ": " + ToString(it->second);
				result += " }";
				return result;
			}
		};

		template<typename ObjectType>
		struct TypeToStringSerializer<ObjectType, TypeToStringObjectType::Enumerable>
		{
			template <typename T>
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
		struct TypeToStringSerializer<ObjectType, TypeToStringObjectType::ProxyObjToStdStream>
		{
			static std::string ToStringImpl(const ObjectType& val)
			{
				std::ostringstream s;
				s << val;
				return s.str();
			}
		};

		template<>
		struct TypeToStringSerializer<u8, TypeToStringObjectType::Other>
		{
			static std::string ToStringImpl(u8 val)
			{
				std::ostringstream s;
				s << (int)val;
				return s.str();
			}
		};

		template<>
		struct TypeToStringSerializer<std::string, TypeToStringObjectType::HasBeginEnd>
		{
			static std::string ToStringImpl(const std::string& str)
			{ return str; }
		};

		template<>
		struct TypeToStringSerializer<const char*, TypeToStringObjectType::Other>
		{
			static std::string ToStringImpl(const char* str)
			{ return str; }
		};

		template<typename T>
		struct TypeToStringSerializer<shared_ptr<T>, TypeToStringObjectType::Other>
		{
			static std::string ToStringImpl(const shared_ptr<T>& ptr)
			{ return ptr ? ToString(*ptr) : "null"; }
		};

		template<typename T>
		struct TypeToStringSerializer<optional<T>, TypeToStringObjectType::Other>
		{
			static std::string ToStringImpl(const optional<T>& opt)
			{ return opt ? ToString(opt.get()) : "null"; }
		};

		template<typename U, typename V>
		struct TypeToStringSerializer<std::pair<U, V>, TypeToStringObjectType::Other>
		{
			static std::string ToStringImpl(const std::pair<U, V>& p)
			{ return "[ " + ToString(p.first) + ", " + ToString(p.second) + " ]"; }
		};

		template<typename ObjectType>
		struct TypeToStringSerializer<ObjectType, TypeToStringObjectType::IsException>
		{
			static std::string ToStringImpl(const ObjectType& object)
			{ return diagnostic_information(object); }
		};

		template<typename ObjectType>
		struct TypeToStringSerializer<ObjectType, TypeToStringObjectType::HasToString>
		{
			static std::string ToStringImpl(const ObjectType& object)
			{ return object.ToString(); }
		};

	}

	template < typename T >
	std::string ToString(const T& val)
	{ return Detail::TypeToStringSerializer<T>::ToStringImpl(val); }


	template < typename T, Detail::TypeToStringObjectType::Enum ObjType = Detail::TypeToStringObjectTypeGetter<T>::Value >
	struct IsStringRepresentable
	{ static const bool Value = true; };

	template < typename T >
	struct IsStringRepresentable<T, Detail::TypeToStringObjectType::ProxyObjToStdStream >
	{ static const bool Value = TypeListContains<BuiltinTypes, T>::Value; }; // TODO: Is this enough?


	/////////////////////////////////////////////////////////////////


	template < typename CharType >
	std::basic_string<CharType>& ReplaceAll(std::basic_string<CharType>& str,
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
		return str;
	}

	inline std::string& ReplaceAll(std::string& str, const std::string& replaceSeq, const std::string& replaceTo)
	{ return ReplaceAll<char>(str, replaceSeq, replaceTo); }

	inline std::string& ReplaceAll(std::string& str, char from, char to)
	{
		for(std::string::iterator it = str.begin(); it != str.end(); ++it)
			if (*it == from)
				*it = to;
		return str;
	}

	inline std::string Filter(const std::string& str, const std::string& characters)
	{
		std::string result;
		for (std::string::const_iterator it = str.begin(); it != str.end(); ++it)
			if (characters.find(*it) == std::string::npos)
				result.push_back(*it);
		return result;
	}

	inline std::string Remove(const std::string& str, char ch = ' ')
	{
		std::string result(str);
		result.erase(std::remove(result.begin(), result.end(), ch), result.end());
		return result;
	}

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

	template<typename ContainerType>
	inline void Split(const std::string& str, const std::string& delim, ContainerType& result)
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
		return pos == std::string::npos? "" : str.substr(0, pos + 1);
	}

	inline std::string LeftStrip(const std::string& str, char ch = ' ')
	{
		const size_t pos = str.find_first_not_of(ch);
		return pos == std::string::npos? "" : str.substr(pos);
	}

	inline std::string Strip(const std::string& str, char ch = ' ')
	{ return LeftStrip(RightStrip(str, ch), ch); }

	template< typename Transformer >
	std::string Transform(const std::string& str, Transformer transformer)
	{
		std::string result;
		std::transform(str.begin(), str.end(), std::back_inserter(result), transformer);
		return result;
	}

	std::string Utf8ToLower(const std::string& str);

	inline std::string ToLower(const std::string& str)
	{ return Transform(str, tolower); }

	inline std::string ToUpper(const std::string& str)
	{ return Transform(str, toupper); }

	inline std::string LeftJustify(const std::string& str, size_t width, char filler = ' ')
	{ return std::string(str.length() < width? width - str.length() : 0, filler) + str; }

	inline std::string RightJustify(const std::string& str, size_t width, char filler = ' ')
	{ return str + std::string(str.length() < width? width - str.length() : 0, filler);  }


	template< typename CharType >
	class BasicStringBuilder
	{
		typedef std::basic_ostringstream<CharType>	StreamType;
		typedef std::basic_string<CharType>			StringType;

	private:
		StreamType	_stream;

	public:
		BasicStringBuilder()
		{ _stream << std::boolalpha; }

		template<typename ObjectType>
		typename EnableIf<!IsIntType<ObjectType>::Value, BasicStringBuilder&>::ValueT operator % (const ObjectType& object)
		{
			_stream << stingray::ToString(object);
			return *this;
		}

		template<typename T>
		typename EnableIf<IsIntType<T>::Value, BasicStringBuilder&>::ValueT operator % (T object)
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
