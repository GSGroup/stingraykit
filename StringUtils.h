#ifndef STINGRAY_TOOLKIT_STRINGUTILS_H
#define STINGRAY_TOOLKIT_STRINGUTILS_H


#include <algorithm>
#include <ctype.h>
#include <string>

#include <stingray/toolkit/collection/CollectionBuilder.h>
#include <stingray/toolkit/Dummy.h>
#include <stingray/toolkit/exception.h>
#include <stingray/toolkit/collection/IEnumerable.h>
#include <stingray/toolkit/Macro.h>
#include <stingray/toolkit/NestedTypeCheck.h>
#include <stingray/toolkit/optional.h>
#include <stingray/toolkit/shared_ptr.h>
#include <stingray/toolkit/string_stream.h>
#include <stingray/toolkit/Types.h>


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
	void ToHexImpl(string_ostream &r, T value, size_t width = 0, bool capital = false)
	{
		static const size_t max_width = sizeof(T) * 2;
		size_t start;
		if (width > max_width)
		{
			for(size_t i = max_width; i < width; ++i)
				r << "0";
			start = 0;
		}
		else
			start = max_width - width;

		bool seen_non_zero = false;
		for(size_t i = 0; i < max_width; ++i)
		{
			char c = (value >> ((max_width - i - 1) * 4)) & 0x0f;
			seen_non_zero |= c;
			if (seen_non_zero || i >= start || i == max_width - 1)
				r << ((char)(c > 9? c + (capital? 'A': 'a') - 10: c + '0'));
		}
	}


	template<typename T>
	std::string ToHex(T value, size_t width = 0, bool capital = false, bool add0xPrefix = false)
	{
		string_ostream result;
		if (add0xPrefix)
			result << "0x";
		ToHexImpl(result, value, width, capital);
		return result.str();
	}


	template < typename T >
	T FromString(const std::string& str)
	{
		if (str.empty()) //old from string behaved this way.
			return 0;

		T value = (T)0;
		bool negative = false;

		size_t i = 0;
		negative = str[0] == '-';
		if (str[0] == '+' || negative)
		{
			++i; //skip first + or -
		}
		for(; i < str.size(); ++i)
		{
			char c = str[i];
			if (c >= '0' && c <= '9')
				value = value * 10 + (c - '0');
			else
				TOOLKIT_THROW(ArgumentException("str", str));
		}

		return negative? (T)0 - value: value; //Dima told me to shut compiler up. Sorry.
	}


	TOOLKIT_DECLARE_METHOD_CHECK(FromString);


	template < typename T >
	void ToString(string_ostream & result, const T& val);


	TOOLKIT_DECLARE_METHOD_CHECK(ToString);


	namespace Detail
	{

		template< typename To, typename From >
		struct LexicalCast;


		template< typename To >
		struct LexicalCast<To, std::string>
		{
			static To Do(const std::string& from)
			{ return FromString<To>(from); }
		};


		template< typename From >
		struct LexicalCast<std::string, From>
		{
			static std::string Do(const From& from)
			{ return ToString(from); }
		};


		template < >
		struct LexicalCast<std::string, std::string>
		{
			static std::string Do(const std::string& from)
			{ return from; }
		};

	}


	template < typename To, typename From >
	To lexical_cast(const From& from)
	{ return Detail::LexicalCast<To, From>::Do(from); }


	namespace Detail
	{

		template < typename From >
		class LexicalCasterProxy
		{
		private:
			From	_from;

		public:
			explicit LexicalCasterProxy(const From& from)
				: _from(from)
			{ }

			template < typename To >
			operator To() const
			{ return lexical_cast<To>(_from); }
		};

	}


	template < typename From >
	Detail::LexicalCasterProxy<From> lexical_caster(const From& from)
	{ return Detail::LexicalCasterProxy<From>(from); }


	namespace Detail
	{

		TOOLKIT_DECLARE_METHOD_CHECK(begin);
		TOOLKIT_DECLARE_METHOD_CHECK(end);


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
											|| IsSharedPtr<ObjectType>::Value
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
			static void ToStringImpl(string_ostream & result, const ObjectType& object)
			{
				typename ObjectType::const_iterator it = object.begin(), iend = object.end();
				result << "[";
				if (it != iend)
					ToString(result, *it++);
				while (it != iend)
				{
					result << ", ";
					ToString(result, *it++);
				}
				result << "]";
			}
		};


		template< typename KeyType, typename ValueType, typename CompareType, typename AllocatorType >
		struct TypeToStringSerializer<std::map<KeyType, ValueType, CompareType, AllocatorType>, TypeToStringObjectType::HasBeginEnd>
		{
			typedef std::map<KeyType, ValueType, CompareType, AllocatorType>	MapType;
			static void ToStringImpl(string_ostream & result, const MapType& object)
			{
				typename MapType::const_iterator it = object.begin(), iend = object.end();
				result << "{ ";
				if (it != iend)
				{
					ToString(result, it->first);
					result << ": ";
					ToString(result, it->second);
					++it;
				}
				for (; it != iend; ++it)
				{
					result << ", ";
					ToString(result, it->first);
					result << ": ";
					ToString(result, it->second);
				}
				result << " }";
			}
		};


		template<typename ObjectType>
		struct TypeToStringSerializer<ObjectType, TypeToStringObjectType::Enumerable>
		{
			template <typename T>
			static void ToStringImpl(string_ostream & result, const IEnumerable<T>& enumerable)
			{
				shared_ptr<IEnumerator<T> > e = TOOLKIT_REQUIRE_NOT_NULL(enumerable.GetEnumerator());
				result << "[";
				if (e->Valid())
				{
					ToString(result, e->Get());
					e->Next();
				}
				while (e->Valid())
				{
					result << ", ";
					ToString(result, e->Get());
					e->Next();
				}
				result << "]";
			}
		};


		template< typename ObjectType>
		struct TypeToStringSerializer<ObjectType, TypeToStringObjectType::ProxyObjToStdStream>
		{
			static void ToStringImpl(string_ostream & result, const ObjectType& val)
			{
				result << val;
			}
		};


		template<>
		struct TypeToStringSerializer<u8, TypeToStringObjectType::Other>
		{
			static void ToStringImpl(string_ostream & result, u8 val)
			{ result << (u16)val; }
		};


		template<>
		struct TypeToStringSerializer<std::string, TypeToStringObjectType::HasBeginEnd>
		{
			static void ToStringImpl(string_ostream & result, const std::string& str)
			{ result << str; }
		};


		template<>
		struct TypeToStringSerializer<const char*, TypeToStringObjectType::Other>
		{
			static void ToStringImpl(string_ostream & result, const char* str)
			{ result << str; }
		};


		template<typename T>
		struct TypeToStringSerializer<shared_ptr<T>, TypeToStringObjectType::Other>
		{
			static void ToStringImpl(string_ostream & result, const shared_ptr<T>& ptr)
			{
				if (ptr)
					ToString(result, *ptr);
				else
					result << "null";
			}
		};


		template<typename T>
		struct TypeToStringSerializer<optional<T>, TypeToStringObjectType::Other>
		{
			static void ToStringImpl(string_ostream & result, const optional<T>& opt)
			{
				if (opt)
					ToString(result, opt.get());
				else
					result << "null";
			}
		};


		template<typename U, typename V>
		struct TypeToStringSerializer<std::pair<U, V>, TypeToStringObjectType::Other>
		{
			static void ToStringImpl(string_ostream & result, const std::pair<U, V>& p)
			{
				result << "[ ";
				ToString(result, p.first);
				result << ", ";
				ToString(result, p.second);
				result << " ]";
			}
		};


		template<typename ObjectType>
		struct TypeToStringSerializer<ObjectType, TypeToStringObjectType::IsException>
		{
			static void ToStringImpl(string_ostream & result, const ObjectType& object)
			{ return diagnostic_information(result, object); }
		};


		template<typename ObjectType>
		struct TypeToStringSerializer<ObjectType, TypeToStringObjectType::HasToString>
		{
			static void ToStringImpl(string_ostream & result, const ObjectType& object)
			{ result << object.ToString(); }
		};

	}


	template < typename T >
	void ToString(string_ostream & result, const T& val)
	{ Detail::TypeToStringSerializer<T>::ToStringImpl(result, val); }


	template < typename T >
	std::string ToString(const T& val)
	{ string_ostream result; ToString(result, val); return result.str(); }


	inline std::string ToString(const EmptyType &)
	{ return std::string(); }


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


	inline std::string RemovePrefix(const std::string& str, const std::string& prefix)
	{ return str.compare(0, prefix.length(), prefix) == 0? str.substr(prefix.length()) : str; }


	inline std::string RemoveSuffix(const std::string& str, const std::string& suffix)
	{
		if (str.length() < suffix.length())
			return str;
		return str.compare(str.length() - suffix.length(), suffix.length(), suffix) == 0? str.substr(0, str.length() - suffix.length()) : str;
	}


	inline bool BeginsWith(const std::string& str, const std::string& prefix)
	{ return str.length() >= prefix.length() && ExtractPrefix(str, prefix.length()) == prefix; }


	inline bool EndsWith(const std::string& str, const std::string& suffix)
	{ return str.length() >= suffix.length() && ExtractSuffix(str, suffix.length()) == suffix; }


	namespace Detail
	{

		class StringRef
		{
		public:
			typedef std::string::size_type	size_type;

			static const size_type npos = std::string::npos;

		private:
			const std::string *		_owner;
			std::string::size_type	_begin;
			std::string::size_type	_end;

		public:
			inline StringRef(const std::string& owner, size_t begin = 0, size_t end = npos) : _owner(&owner), _begin(begin), _end(end != npos ? end : owner.size()) { }

			inline bool empty() const		{ return _begin >= _end; }
			inline size_type size() const	{ return _end >= _begin ? _end - _begin : 0; }

			inline size_type find(const char c, size_type pos = 0) const
			{
				size_type p = _owner->find(c, pos + _begin);
				return (p >= _end) ? npos : p - _begin;
			}

			inline size_type find(const std::string& str, size_type pos = 0) const
			{
				size_type p = _owner->find(str, pos + _begin);
				return (p >= _end) ? npos : p - _begin;
			}

			inline std::string substr(size_type pos = 0, size_type n = npos) const
			{
				return pos < size() ? _owner->substr(_begin + pos, std::min(size() - pos, n)) : std::string();
			}

			inline std::string str() const { return substr(); }
		};


		template<typename ContainerType>
		inline void SplitRefsImpl(const std::string& sourceString, const std::vector<std::string>& delimiters, ContainerType& result, int maxsplit)
		{
			size_t lastPosition = 0;
			size_t delimiterPosition;
			int counter = 0;
			do
			{
				delimiterPosition = std::string::npos;
				size_t delimiterSize = 0;

				for (size_t i = 0; i < delimiters.size(); ++i)
				{
					size_t position;
					if ((position = sourceString.find(delimiters[i], lastPosition)) != std::string::npos && (delimiterPosition == std::string::npos || position < delimiterPosition))
					{
						delimiterPosition = position;
						delimiterSize = delimiters[i].length();
					}
				}

				if (delimiterPosition != std::string::npos && (maxsplit < 0 || counter < maxsplit))
				{
					result.push_back(StringRef(sourceString, lastPosition, delimiterPosition));
					lastPosition = delimiterPosition + delimiterSize;
					++counter;
				}
			}
			while (delimiterPosition != std::string::npos && (maxsplit < 0 || counter < maxsplit));

			result.push_back(StringRef(sourceString, lastPosition));
		}


		template<typename ContainerType, typename UnaryOperator, typename ValueType>
		struct SplitImpl
		{
			void operator()(const std::string& sourceString, const std::vector<std::string>& delimiters, ContainerType& result, UnaryOperator op, int maxsplit)
			{
				std::vector<StringRef> refs;
				SplitRefsImpl(sourceString, delimiters, refs, maxsplit);

				for (size_t i = 0; i < refs.size(); ++i)
					Detail::CollectionInserter<ContainerType>::Insert(result, op(refs[i].str()));
			}
		};

	}


	typedef Detail::StringRef	StringRef;


#define DETAIL_SPLIT_PARAM_USAGE(Index_, UserArg_) % TOOLKIT_CAT(p, Index_)

#define DETAIL_TOOLKIT_DECLARE_SPLIT_FUNCTION(ParamsCount_, UserData_) \
	TOOLKIT_INSERT_IF(ParamsCount_, \
		template <typename ContainerType> \
		inline void Split(const std::string& str, TOOLKIT_REPEAT(ParamsCount_, TOOLKIT_FUNCTION_TYPED_PARAM_DECL, std::string), ContainerType& result, int maxsplit = -1, Detail::SplitImpl<ContainerType, typename ContainerType::value_type(*)(const std::string&), typename ContainerType::value_type> splitImpl = Detail::SplitImpl<ContainerType, typename ContainerType::value_type(*)(const std::string&), typename ContainerType::value_type>()) \
		{ \
			splitImpl(str, VectorBuilder<std::string>() TOOLKIT_REPEAT(ParamsCount_, DETAIL_SPLIT_PARAM_USAGE, TOOLKIT_EMPTY()), result, lexical_cast<typename ContainerType::value_type, std::string>, maxsplit); \
		} \
	) \
	TOOLKIT_INSERT_IF(ParamsCount_, \
		template <typename ContainerType TOOLKIT_COMMA typename UnaryOperator> \
		inline void Split(const std::string& str, TOOLKIT_REPEAT(ParamsCount_, TOOLKIT_FUNCTION_TYPED_PARAM_DECL, std::string), ContainerType& result, UnaryOperator op, int maxsplit = -1, Detail::SplitImpl<ContainerType, UnaryOperator, typename ContainerType::value_type> splitImpl = Detail::SplitImpl<ContainerType, UnaryOperator, typename ContainerType::value_type>()) \
		{ \
			splitImpl(str, VectorBuilder<std::string>() TOOLKIT_REPEAT(ParamsCount_, DETAIL_SPLIT_PARAM_USAGE, TOOLKIT_EMPTY()), result, op, maxsplit); \
		} \
	)

	TOOLKIT_REPEAT_NESTING_2(10, DETAIL_TOOLKIT_DECLARE_SPLIT_FUNCTION, TOOLKIT_EMPTY())

#define DETAIL_TOOLKIT_DECLARE_SPLITREFS_FUNCTION(ParamsCount_, UserData_) \
	TOOLKIT_INSERT_IF(ParamsCount_, \
		template <typename ContainerType> \
		inline void SplitRefs(const std::string& str, TOOLKIT_REPEAT(ParamsCount_, TOOLKIT_FUNCTION_TYPED_PARAM_DECL, std::string), ContainerType& result, int maxsplit = -1) \
		{ \
			Detail::SplitRefsImpl(str, VectorBuilder<std::string>() TOOLKIT_REPEAT(ParamsCount_, DETAIL_SPLIT_PARAM_USAGE, TOOLKIT_EMPTY()), result, maxsplit); \
		} \
	)

	TOOLKIT_REPEAT_NESTING_2(10, DETAIL_TOOLKIT_DECLARE_SPLITREFS_FUNCTION, TOOLKIT_EMPTY())

#undef DETAIL_SPLIT_PARAM_USAGE
#undef DETAIL_TOOLKIT_DECLARE_SPLIT_FUNCTION
#undef DETAIL_TOOLKIT_DECLARE_SPLITREFS_FUNCTION


	template < typename InputIterator, typename UnaryOperator >
	std::string Join(const std::string& separator, InputIterator first, InputIterator last, UnaryOperator op)
	{
		std::string result;
		while (first != last)
		{
			if (!result.empty())
				result.append(separator);

			result.append(op(*first));
			++first;
		}
		return result;
	}


	template < typename InputIterator >
	std::string Join(const std::string& separator, InputIterator first, InputIterator last)
	{ return Join(separator, first, last, lexical_cast<std::string, typename std::iterator_traits<InputIterator>::value_type>); }


	inline std::string RightStrip(const std::string& str, const std::string& chars = " \t\n\r\f\v")
	{
		const size_t pos = str.find_last_not_of(chars);
		return pos == std::string::npos? "" : str.substr(0, pos + 1);
	}


	inline std::string LeftStrip(const std::string& str, const std::string& chars = " \t\n\r\f\v")
	{
		const size_t pos = str.find_first_not_of(chars);
		return pos == std::string::npos? "" : str.substr(pos);
	}


	inline std::string Strip(const std::string& str, const std::string& chars = " \t\n\r\f\v")
	{ return LeftStrip(RightStrip(str, chars), chars); }


	template< typename Transformer >
	std::string Transform(const std::string& str, Transformer transformer)
	{
		string_ostream result;
		std::transform(str.begin(), str.end(), std::back_inserter(result), transformer);
		return result.str();
	}


	inline std::string ToLower(const std::string& str)
	{ return Transform(str, ::tolower); }


	inline std::string ToUpper(const std::string& str)
	{ return Transform(str, ::toupper); }


	inline std::string Capitalize(const std::string& str)
	{ return str.empty()? str : ToUpper(str.substr(0, 1)) + str.substr(1); }


	inline std::string Uncapitalize(const std::string& str)
	{ return str.empty()? str : ToLower(str.substr(0, 1)) + str.substr(1); }


	inline std::string LeftJustify(const std::string& str, size_t width, char filler = ' ')
	{ return str + std::string(str.length() < width? width - str.length() : 0, filler); }


	inline std::string RightJustify(const std::string& str, size_t width, char filler = ' ')
	{ return std::string(str.length() < width? width - str.length() : 0, filler) + str;  }


	template< typename CharType >
	class BasicStringBuilder
	{
		typedef basic_string_ostream<CharType>		StreamType;
		typedef std::basic_string<CharType>			StringType;

	private:
		StreamType	_stream;

	public:
		BasicStringBuilder()
		{ }

		template<typename ObjectType>
		typename EnableIf<!IsIntType<ObjectType>::Value, BasicStringBuilder&>::ValueT operator % (const ObjectType& object)
		{
			stingray::ToString(_stream, object);
			return *this;
		}

		template<typename T>
		typename EnableIf<IsIntType<T>::Value, BasicStringBuilder&>::ValueT operator % (T object)
		{
			stingray::ToString(_stream, object);
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


#endif
