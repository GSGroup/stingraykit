#ifndef STINGRAYKIT_STRING_TOSTRING_H
#define STINGRAYKIT_STRING_TOSTRING_H

// Copyright (c) 2011 - 2019, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/collection/IEnumerable.h>
#include <stingraykit/string/string_stream.h>
#include <stingraykit/optional.h>

#include <algorithm>
#include <ctype.h>
#include <limits>

namespace stingray
{

	STINGRAYKIT_DECLARE_METHOD_CHECK(FromString);

	namespace Detail
	{

		template < typename T >
		typename EnableIf<!std::numeric_limits<T>::is_specialized, T>::ValueT EvaluateHelper(char c, const T& value, bool)
		{ return value * 10 + (c - '0'); }

		template < typename T >
		typename EnableIf<std::numeric_limits<T>::is_specialized && std::numeric_limits<T>::is_signed, T>::ValueT EvaluateHelper(char c, const T& value, bool negative)
		{
			const s64 newValue = value * (s64) 10 + (c - '0');
			if (negative)
				STINGRAYKIT_CHECK(((0 - newValue) >= (s64) std::numeric_limits<T>::min()), IndexOutOfRangeException(0 - newValue, std::numeric_limits<T>::min()));
			else
				STINGRAYKIT_CHECK((newValue <= (s64) std::numeric_limits<T>::max()), IndexOutOfRangeException(newValue, std::numeric_limits<T>::max()));
			return value * 10 + (c - '0');
		}

		template < typename T >
		typename EnableIf<std::numeric_limits<T>::is_specialized && !std::numeric_limits<T>::is_signed, T>::ValueT EvaluateHelper(char c, const T& value, bool negative)
		{
			STINGRAYKIT_CHECK(!negative, "Value cannot be negative!");
			const u64 newValue = value * (u64) 10 + (c - '0');
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
	typename EnableIf<!HasMethod_FromString<T>::Value && !IsSame<T, StringType>::Value, T>::ValueT FromString(const StringType& str)
	{
		if (str.empty()) //old from string behaved this way.
			return 0;

		T value = (T)0;
		bool negative = false;

		size_t index = 0;
		negative = str[0] == '-';
		if (negative || str[0] == '+')
			++index;

		for (; index < str.size(); ++index)
		{
			const char c = str[index];
			STINGRAYKIT_CHECK(c >= '0' && c <= '9', ArgumentException("str", str));

			value = Detail::EvaluateHelper(c, value, negative);
		}

		return negative ? (T)0 - value : value; //Dima told me to shut compiler up. Sorry.
	}

	template < typename T, typename StringType >
	typename EnableIf<IsSame<T, StringType>::Value, StringType>::ValueT FromString(const StringType& str)
	{ return str; }


	template < typename T >
	void ToString(string_ostream& result, const T& val);


	STINGRAYKIT_DECLARE_METHOD_CHECK(ToString);


	namespace Detail
	{

		STINGRAYKIT_DECLARE_METHOD_CHECK(begin);
		STINGRAYKIT_DECLARE_METHOD_CHECK(end);

		STINGRAYKIT_DECLARE_NESTED_TYPE_CHECK(mapped_type);


		struct TypeToStringObjectType
		{
			STINGRAYKIT_ENUM_VALUES(HasToString, Range, Enumerable, IsMap, HasBeginEnd, IsException, Other, ProxyObjToStdStream);
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
								(HasNestedType_mapped_type<ObjectType>::Value ?
									TypeToStringObjectType::IsMap :
									(HasMethod_begin<ObjectType>::Value && HasMethod_end<ObjectType>::Value ?
										TypeToStringObjectType::HasBeginEnd :
										(IsInherited<ObjectType, std::exception>::Value ?
											TypeToStringObjectType::IsException :
											(
												IsSame<u8, ObjectType>::Value
													|| IsSame<EmptyType, ObjectType>::Value
													|| IsSame<NullPtrType, ObjectType>::Value
													|| IsSame<const char*, ObjectType>::Value
													|| IsSharedPtr<ObjectType>::Value
													|| IsOptional<ObjectType>::Value
													|| Is1ParamTemplate<Tuple, ObjectType>::Value
													|| Is2ParamTemplate<std::pair, ObjectType>::Value ?
												TypeToStringObjectType::Other :
												TypeToStringObjectType::ProxyObjToStdStream
											)
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


		template < typename ObjectType >
		struct TypeToStringSerializer<ObjectType, TypeToStringObjectType::HasBeginEnd>
		{
			static void ToStringImpl(string_ostream& result, const ObjectType& object)
			{
				typename ObjectType::const_iterator it = object.begin();
				const typename ObjectType::const_iterator iend = object.end();

				result << "[";
				if (it != iend)
				{
					ToString(result, *it);
					++it;
				}

				for (; it != iend; ++it)
				{
					result << ", ";
					ToString(result, *it);
				}
				result << "]";
			}
		};


		template < typename ObjectType >
		struct TypeToStringSerializer<ObjectType, TypeToStringObjectType::IsMap>
		{
			static void ToStringImpl(string_ostream& result, const ObjectType& object)
			{
				typename ObjectType::const_iterator it = object.begin();
				const typename ObjectType::const_iterator iend = object.end();

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


		template < typename ObjectType >
		struct TypeToStringSerializer<ObjectType, TypeToStringObjectType::Enumerable>
		{
			template < typename T >
			static void ToStringImpl(string_ostream& result, const IEnumerable<T>& enumerable)
			{
				const shared_ptr<IEnumerator<T>> en = STINGRAYKIT_REQUIRE_NOT_NULL(enumerable.GetEnumerator());

				result << "[";
				if (en->Valid())
				{
					ToString(result, en->Get());
					en->Next();
				}

				for (; en->Valid(); en->Next())
				{
					result << ", ";
					ToString(result, en->Get());
				}
				result << "]";
			}
		};


		template < typename ObjectType >
		struct TypeToStringSerializer<ObjectType, TypeToStringObjectType::Range>
		{
			static void ToStringImpl(string_ostream& result, const ObjectType& range)
			{
				ObjectType copy(range);

				result << "[";
				if (copy.Valid())
				{
					ToString(result, copy.Get());
					copy.Next();
				}

				for (; copy.Valid(); copy.Next())
				{
					result << ", ";
					ToString(result, copy.Get());
				}
				result << "]";
			}
		};


		template < typename ObjectType >
		struct TypeToStringSerializer<ObjectType, TypeToStringObjectType::ProxyObjToStdStream>
		{
			static void ToStringImpl(string_ostream& result, const ObjectType& val)
			{ result << val; }
		};


		template<>
		struct TypeToStringSerializer<u8, TypeToStringObjectType::Other>
		{
			static void ToStringImpl(string_ostream& result, u8 val)
			{ result << (u16)val; }
		};


		template<>
		struct TypeToStringSerializer<std::string, TypeToStringObjectType::HasBeginEnd>
		{
			static void ToStringImpl(string_ostream& result, const std::string& str)
			{ result << str; }
		};


		template<>
		struct TypeToStringSerializer<string_view, TypeToStringObjectType::HasBeginEnd>
		{
			static void ToStringImpl(string_ostream & result, string_view str)
			{ result << str; }
		};


		template<>
		struct TypeToStringSerializer<EmptyType, TypeToStringObjectType::Other>
		{
			static void ToStringImpl(string_ostream& result, EmptyType val)
			{ }
		};


		template<>
		struct TypeToStringSerializer<NullPtrType, TypeToStringObjectType::Other>
		{
			static void ToStringImpl(string_ostream& result, NullPtrType ptr)
			{ result << "null"; }
		};


		template<>
		struct TypeToStringSerializer<const char*, TypeToStringObjectType::Other>
		{
			static void ToStringImpl(string_ostream& result, const char* str)
			{ result << str; }
		};


		template < typename T >
		struct TypeToStringSerializer<shared_ptr<T>, TypeToStringObjectType::Other>
		{
			static void ToStringImpl(string_ostream& result, const shared_ptr<T>& ptr)
			{
				if (ptr)
					ToString(result, *ptr);
				else
					result << "null";
			}
		};


		template < typename T >
		struct TypeToStringSerializer<optional<T>, TypeToStringObjectType::Other>
		{
			static void ToStringImpl(string_ostream& result, const optional<T>& opt)
			{
				if (opt)
					ToString(result, *opt);
				else
					result << "null";
			}
		};


		template < typename Types >
		struct TypeToStringSerializer<Tuple<Types>, TypeToStringObjectType::Other>
		{
			template < size_t Index >
			struct Helper
			{
				static void Call(string_ostream* result, const Tuple<Types>* tuple)
				{
					if (Index != 0)
						*result << ", ";

					ToString(*result, tuple->template Get<Index>());
				}
			};

			static void ToStringImpl(string_ostream& result, const Tuple<Types>& tuple)
			{
				result << "[ ";
				For<GetTypeListLength<Types>::Value, Helper>::Do(&result, &tuple);
				result << " ]";
			}
		};


		template < typename K, typename V >
		struct TypeToStringSerializer<std::pair<K, V>, TypeToStringObjectType::Other>
		{
			static void ToStringImpl(string_ostream& result, const std::pair<K, V>& pair)
			{
				result << "[ ";
				ToString(result, pair.first);
				result << ", ";
				ToString(result, pair.second);
				result << " ]";
			}
		};


		template < typename ObjectType >
		struct TypeToStringSerializer<ObjectType, TypeToStringObjectType::IsException>
		{
			static void ToStringImpl(string_ostream& result, const ObjectType& object)
			{ return diagnostic_information(result, object); }
		};


		template < typename ObjectType >
		struct TypeToStringSerializer<ObjectType, TypeToStringObjectType::HasToString>
		{
			static void ToStringImpl(string_ostream& result, const ObjectType& object)
			{ result << object.ToString(); }
		};


		template < typename T, Detail::TypeToStringObjectType::Enum ObjType = Detail::TypeToStringObjectTypeGetter<T>::Value >
		struct IsStringRepresentableImpl : TrueType { };


		template < typename T >
		struct IsStringRepresentableImpl<T, Detail::TypeToStringObjectType::Enumerable> : IsStringRepresentableImpl<typename T::ItemType> { };


		template < typename T >
		struct IsStringRepresentableImpl<T, Detail::TypeToStringObjectType::HasBeginEnd> : IsStringRepresentableImpl<typename T::value_type> { };


		template < >
		struct IsStringRepresentableImpl<EmptyType, Detail::TypeToStringObjectType::Other> : TrueType { };


		template < typename T >
		struct IsStringRepresentableImpl<shared_ptr<T>, Detail::TypeToStringObjectType::Other> : IsStringRepresentableImpl<T> { };


		template < typename T >
		struct IsStringRepresentableImpl<optional<T>, Detail::TypeToStringObjectType::Other> : IsStringRepresentableImpl<T> { };


		template < typename T, typename U >
		struct IsStringRepresentableImpl<std::pair<T, U>, Detail::TypeToStringObjectType::Other> : integral_constant<bool, IsStringRepresentableImpl<T>::Value && IsStringRepresentableImpl<U>::Value> { };


		template < typename T >
		struct IsStringRepresentableImpl<T, Detail::TypeToStringObjectType::ProxyObjToStdStream > : TypeListContains<BuiltinTypes, T> { }; // TODO: Is this enough?

	}


	template < typename T >
	void ToString(string_ostream& result, const T& val)
	{ Detail::TypeToStringSerializer<T>::ToStringImpl(result, val); }


	template < typename T >
	std::string ToString(const T& val)
	{
		string_ostream result;
		ToString(result, val);
		return result.str();
	}


	template < typename T >
	struct IsStringRepresentable : Detail::IsStringRepresentableImpl<T> { };


	template < typename CharType >
	class BasicStringBuilder
	{
		using StreamType = basic_string_ostream<CharType>;
		using StringType = std::basic_string<CharType>;

	private:
		StreamType	_stream;

	public:
		BasicStringBuilder()
		{ }

		template < typename ObjectType >
		typename EnableIf<!IsInt<ObjectType>::Value, BasicStringBuilder&>::ValueT operator % (const ObjectType& object)
		{
			stingray::ToString(_stream, object);
			return *this;
		}

		template < typename T >
		typename EnableIf<IsInt<T>::Value, BasicStringBuilder&>::ValueT operator % (T object)
		{
			stingray::ToString(_stream, object);
			return *this;
		}

		bool empty() const { return _stream.empty(); }

		operator StringType () const
		{ return _stream.str(); }

		std::string ToString() const
		{ return _stream.str(); }
	};


	using StringBuilder = BasicStringBuilder<char>;
	using WideStringBuilder = BasicStringBuilder<wchar_t>;


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
		StringJoiner& operator % (const T& value)
		{
			if (_builder.empty())
				_builder % _prefix;
			else
				_builder % _separator;

			_builder % value;
			return *this;
		}

		bool empty() const { return _builder.empty(); }

		operator std::string () const { return ToString(); }

		std::string ToString() const
		{ return (_builder.empty() ? _prefix : _builder.ToString()) + _suffix; }
	};

}

#endif
