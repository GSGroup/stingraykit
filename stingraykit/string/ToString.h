#ifndef STINGRAYKIT_STRING_TOSTRING_H
#define STINGRAYKIT_STRING_TOSTRING_H

// Copyright (c) 2011 - 2017, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/Dummy.h>
#include <stingraykit/Macro.h>
#include <stingraykit/collection/CollectionBuilder.h>
#include <stingraykit/collection/IEnumerable.h>
#include <stingraykit/collection/RangeBase.h>
#include <stingraykit/exception.h>
#include <stingraykit/optional.h>
#include <stingraykit/shared_ptr.h>
#include <stingraykit/string/string_stream.h>

#include <algorithm>
#include <ctype.h>
#include <limits>
#include <string>

namespace std
{
	template < class Key, class T, class Compare, class Allocator > class map;
}


namespace stingray
{

	STINGRAYKIT_DECLARE_METHOD_CHECK(FromString);

	namespace Detail
	{

		template < typename T >
		typename EnableIf<!std::numeric_limits<T>::is_specialized, T>::ValueT EvaluateHelper(const char c, const T& value, bool)
		{ return value * 10 + (c - '0'); }

		template < typename T >
		typename EnableIf<std::numeric_limits<T>::is_specialized && std::numeric_limits<T>::is_signed, T>::ValueT EvaluateHelper(const char c, const T& value, bool negative)
		{
			s64 newValue = value * (s64) 10 + (c - '0');
			if (negative)
				STINGRAYKIT_CHECK(((0 - newValue) >= (s64) std::numeric_limits<T>::min()), IndexOutOfRangeException(0 - newValue, std::numeric_limits<T>::min()));
			else
				STINGRAYKIT_CHECK((newValue <= (s64) std::numeric_limits<T>::max()), IndexOutOfRangeException(newValue, std::numeric_limits<T>::max()));
			return value * 10 + (c - '0');
		}

		template < typename T >
		typename EnableIf<std::numeric_limits<T>::is_specialized && !std::numeric_limits<T>::is_signed, T>::ValueT EvaluateHelper(const char c, const T& value, bool negative)
		{
			STINGRAYKIT_CHECK(!negative, "Value cannot be negative!");
			u64 newValue = value * (u64) 10 + (c - '0');
			STINGRAYKIT_CHECK(newValue <= (u64) std::numeric_limits<T>::max(), IndexOutOfRangeException(newValue, std::numeric_limits<T>::max()));
			return value * 10 + (c - '0');
		}

	}

	template < typename T >
	typename EnableIf<HasMethod_FromString<T>::Value, T>::ValueT FromString(const std::string& str)
	{
		return T::FromString(str);
	}

	template < typename T, typename StringType >
	typename EnableIf<!HasMethod_FromString<T>::Value && !IsSame<T, StringType>::Value, T>::ValueT FromString(const StringType & str)
	{
		if (str.empty()) //old from string behaved this way.
			return 0;

		T value = (T)0;
		bool negative = false;

		size_t i = 0;
		negative = str[0] == '-';
		if (str[0] == '+' || negative)
			++i;

		for(; i < str.size(); ++i)
		{
			char c = str[i];
			if (c >= '0' && c <= '9')
				value = Detail::EvaluateHelper(c, value, negative);
			else
				STINGRAYKIT_THROW(ArgumentException("str", str));
		}

		return negative? (T)0 - value: value; //Dima told me to shut compiler up. Sorry.
	}

	template < typename T, typename StringType >
	typename EnableIf<IsSame<T, StringType>::Value, StringType>::ValueT FromString(const StringType & str)
	{ return str; }


	template < typename T >
	void ToString(string_ostream & result, const T& val);


	STINGRAYKIT_DECLARE_METHOD_CHECK(ToString);


	namespace Detail
	{

		STINGRAYKIT_DECLARE_METHOD_CHECK(begin);
		STINGRAYKIT_DECLARE_METHOD_CHECK(end);


		struct TypeToStringObjectType
		{
			STINGRAYKIT_ENUM_VALUES(HasToString, Range, Enumerable, HasBeginEnd, IsException, Other, ProxyObjToStdStream);
			STINGRAYKIT_DECLARE_ENUM_CLASS(TypeToStringObjectType);
		};


		template
			<
				typename ObjectType,
				TypeToStringObjectType::Enum ObjType =
					HasMethod_ToString<ObjectType>::Value ?
						TypeToStringObjectType::HasToString :
						(IsRange<ObjectType>::Value ?
							TypeToStringObjectType::Range :
							(IsEnumerable<ObjectType>::Value ?
								TypeToStringObjectType::Enumerable :
								(HasMethod_begin<ObjectType>::Value && HasMethod_end<ObjectType>::Value ?
									TypeToStringObjectType::HasBeginEnd :
									(IsInherited<ObjectType, std::exception>::Value ?
										TypeToStringObjectType::IsException :
										(
											IsSame<u8, ObjectType>::Value
												|| IsSame<NullPtrType, ObjectType>::Value
												|| IsSame<const char*, ObjectType>::Value
												|| IsSharedPtr<ObjectType>::Value
												|| Is1ParamTemplate<optional, ObjectType>::Value
												|| Is1ParamTemplate<Tuple, ObjectType>::Value
												|| Is2ParamTemplate<std::pair, ObjectType>::Value ?
											TypeToStringObjectType::Other :
											TypeToStringObjectType::ProxyObjToStdStream
										)
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
				shared_ptr<IEnumerator<T> > e = STINGRAYKIT_REQUIRE_NOT_NULL(enumerable.GetEnumerator());
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


		template<typename ObjectType>
		struct TypeToStringSerializer<ObjectType, TypeToStringObjectType::Range>
		{
			static void ToStringImpl(string_ostream & result, const ObjectType& range)
			{
				ObjectType copy(range);
				result << "[";
				if (copy.Valid())
				{
					ToString(result, copy.Get());
					copy.Next();
				}
				while (copy.Valid())
				{
					result << ", ";
					ToString(result, copy.Get());
					copy.Next();
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
		struct TypeToStringSerializer<NullPtrType, TypeToStringObjectType::Other>
		{
			static void ToStringImpl(string_ostream & result, NullPtrType ptr)
			{ result << "null"; }
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


		template<typename Types_>
		struct TypeToStringSerializer<Tuple<Types_>, TypeToStringObjectType::Other>
		{
			template <int Index>
			struct Helper
			{
				static void Call(string_ostream* result, const Tuple<Types_>* t)
				{
					if (Index != 0)
						*result << ", ";

					ToString(*result, t->template Get<Index>());
				}
			};

			static void ToStringImpl(string_ostream & result, const Tuple<Types_>& t)
			{
				result << "[ ";
				For<GetTypeListLength<Types_>::Value, Helper>::Do(&result, &t);
				result << " ]";
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
	struct IsStringRepresentable : TrueType { };


	template < typename T >
	struct IsStringRepresentable<T, Detail::TypeToStringObjectType::Enumerable> : IsStringRepresentable<typename T::ItemType> { };


	template < typename T >
	struct IsStringRepresentable<T, Detail::TypeToStringObjectType::HasBeginEnd> : IsStringRepresentable<typename T::value_type> { };


	template < typename T >
	struct IsStringRepresentable<shared_ptr<T>, Detail::TypeToStringObjectType::Other> : IsStringRepresentable<T> { };


	template < typename T >
	struct IsStringRepresentable<optional<T>, Detail::TypeToStringObjectType::Other> : IsStringRepresentable<T> { };


	template < typename T, typename U >
	struct IsStringRepresentable<std::pair<T, U>, Detail::TypeToStringObjectType::Other> : integral_constant<bool, IsStringRepresentable<T>::Value && IsStringRepresentable<U>::Value> { };


	template < typename T >
	struct IsStringRepresentable<T, Detail::TypeToStringObjectType::ProxyObjToStdStream > : TypeListContains<BuiltinTypes, T> { }; // TODO: Is this enough?


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
		typename EnableIf<!IsInt<ObjectType>::Value, BasicStringBuilder&>::ValueT operator % (const ObjectType& object)
		{
			stingray::ToString(_stream, object);
			return *this;
		}

		template<typename T>
		typename EnableIf<IsInt<T>::Value, BasicStringBuilder&>::ValueT operator % (T object)
		{
			stingray::ToString(_stream, object);
			return *this;
		}

		bool empty() const { return _stream.empty(); }

		operator StringType() const
		{ return _stream.str(); }

		std::string ToString() const
		{ return _stream.str(); }
	};


	typedef BasicStringBuilder<char>	StringBuilder;
	typedef BasicStringBuilder<wchar_t>	WideStringBuilder;


	class StringJoiner
	{
	private:
		StringBuilder	_builder;

		std::string		_separator;

		std::string		_prefix;
		std::string		_suffix;

	public:
		explicit StringJoiner(const std::string& separator)
			: _separator(separator)
		{ }

		StringJoiner(const std::string& separator, const std::string& prefix, const std::string& suffix)
			: _separator(separator), _prefix(prefix), _suffix(suffix)
		{ }

		template < typename T >
		StringJoiner& operator% (const T& value)
		{
			if (!_builder.empty())
				_builder % _separator;
			_builder % value;
			return *this;
		}

		operator std::string() const { return ToString(); }

		std::string ToString() const
		{
			return _prefix + _builder.ToString() + _suffix;
		}
	};

}


#endif
