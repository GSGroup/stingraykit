#ifndef STINGRAYKIT_STRING_TOSTRING_H
#define STINGRAYKIT_STRING_TOSTRING_H

// Copyright (c) 2011 - 2019, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/collection/IEnumerable.h>
#include <stingraykit/string/ToStringForward.h>
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
												IsSame<EmptyType, ObjectType>::Value
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


		template < typename T >
		struct TypeToStringSerializer
		{
		private:
			struct MapItemPrinter
			{
				template < typename K, typename V >
				static auto Do(string_ostream& result, const std::pair<K, V>& pair)
						-> decltype(ToString(result, pair.first), ToString(result, pair.second), void())
				{
					ToString(result, pair.first);
					result << ": ";
					ToString(result, pair.second);
				}
			};

			struct CollectionItemPrinter
			{
				template < typename ObjectType >
				static auto Do(string_ostream& result, const ObjectType& object)
						-> decltype(ToString(result, object))
				{ ToString(result, object); }
			};

			template < size_t Size >
			struct TypeToStringTester
			{
				static void Test(string_ostream& result);

				template < typename U0, typename... Us >
				static auto Test(string_ostream& result, const U0& p0, const Us&... args)
						-> decltype(ToString(result, p0), TypeToStringTester<sizeof...(Us)>::Test(result, args...), void());
			};

			template < typename Types, size_t... Index >
			static auto TestTypeListToString(string_ostream& result, std::index_sequence<Index...>)
					-> decltype(TypeToStringTester<sizeof...(Index)>::Test(result, std::declval<typename GetTypeListItem<Types, Index>::ValueT>()...), void());

			template < size_t Index >
			struct TuplePrinter
			{
				template < typename TupleType >
				static auto Call(string_ostream* result, const TupleType* tuple)
						-> decltype(ToString(*result, tuple->template Get<Index>()), void())
				{
					if (Index == 0)
						*result << " ";
					else
						*result << ", ";

					ToString(*result, tuple->template Get<Index>());
				}
			};

			template < typename Range >
			static void PrintRange(string_ostream& result, Range& range)
			{
				result << "[";
				if (range.Valid())
				{
					ToString(result, range.Get());
					range.Next();
				}

				for (; range.Valid(); range.Next())
				{
					result << ", ";
					ToString(result, range.Get());
				}
				result << "]";
			}

			template < typename ItemPrinter, typename ObjectType, typename EnableIf<!IsSame<ObjectType, typename Decay<decltype(*std::declval<ObjectType>().begin())>::ValueT>::Value, bool>::ValueT = false >
			static auto PrintStdCollection(string_ostream& result, const char* emptyPrefix, const char* prefix, const char* suffix, const ObjectType& items)
					-> decltype(ItemPrinter::Do(result, *items.begin()), void())
			{
				typename ObjectType::const_iterator it = items.begin();
				const typename ObjectType::const_iterator iend = items.end();

				if (it != iend)
				{
					result << prefix;
					ItemPrinter::Do(result, *it);
					++it;
				}
				else
					result << emptyPrefix;

				for (; it != iend; ++it)
				{
					result << ", ";
					ItemPrinter::Do(result, *it);
				}
				result << suffix;
			}

			template < typename ObjectType >
			static auto StdCollectionToString(string_ostream& result, const ObjectType& object, int)
					-> decltype(object.begin(), object.end(), PrintStdCollection<CollectionItemPrinter>(result, std::declval<const char*>(), std::declval<const char*>(), std::declval<const char*>(), object), void())
			{ PrintStdCollection<CollectionItemPrinter>(result, "[", "[", "]", object); }

			template < typename ObjectType >
			static auto StdCollectionToString(string_ostream& result, const ObjectType& object, long)
					-> decltype(result << object, void())
			{ result << object; }

			template < typename ObjectType >
			static auto StdMapToString(string_ostream& result, const ObjectType& object, int)
					-> decltype(std::declval<typename ObjectType::mapped_type>(), PrintStdCollection<MapItemPrinter>(result, std::declval<const char*>(), std::declval<const char*>(), std::declval<const char*>(), object), void())
			{ PrintStdCollection<MapItemPrinter>(result, "{", "{ ", " }", object); }

			template < typename ObjectType >
			static auto StdMapToString(string_ostream& result, const ObjectType& object, long)
					-> decltype(StdCollectionToString(result, object, 0), void())
			{ StdCollectionToString(result, object, 0); }

			template < typename ObjectType >
			static auto SpecialToString(string_ostream& result, const ObjectType& range, typename EnableIf<IsRange<ObjectType>::Value, int>::ValueT)
					-> decltype(ToString(result, std::declval<typename ObjectType::ValueType>()), void())
			{
				ObjectType copy(range);
				PrintRange(result, copy);
			}

			template < typename ObjectType >
			static auto SpecialToString(string_ostream& result, const ObjectType& enumerable, typename EnableIf<IsEnumerable<ObjectType>::Value, int>::ValueT)
					-> decltype(ToString(result, std::declval<typename ObjectType::ItemType>()), void())
			{
				const shared_ptr<IEnumerator<typename ObjectType::ItemType>> en = STINGRAYKIT_REQUIRE_NOT_NULL(enumerable.GetEnumerator());
				PrintRange(result, *en);
			}

			template < typename ObjectType >
			static auto SpecialToString(string_ostream& result, const ObjectType& object, typename EnableIf<IsSharedPtr<ObjectType>::Value || IsOptional<ObjectType>::Value, int>::ValueT)
					-> decltype(ToString(result, *object), void())
			{
				if (object)
					ToString(result, *object);
				else
					result << "null";
			}

			template < typename ObjectType >
			static void SpecialToString(string_ostream& result, const ObjectType& ex, typename EnableIf<IsInherited<ObjectType, std::exception>::Value, int>::ValueT)
			{ diagnostic_information(result, ex); }

			template < typename ObjectType >
			static auto SpecialToString(string_ostream& result, const ObjectType& object, long)
					-> decltype(StdMapToString(result, object, 0), void())
			{ StdMapToString(result, object, 0); }

		public:
			template < typename ObjectType >
			static auto ToStringImpl(string_ostream& result, const ObjectType& object, int)
					-> decltype(object.ToString(), void())
			{ result << object.ToString(); }

			template < typename ObjectType >
			static auto ToStringImpl(string_ostream& result, const ObjectType& object, long)
					-> decltype(SpecialToString(result, object, 0), void())
			{ SpecialToString(result, object, 0); }

			static void ToStringImpl(string_ostream& result, const char* str, long)
			{ result << str; }

			static void ToStringImpl(string_ostream& result, const std::string& str, long)
			{ result << str; }

			static void ToStringImpl(string_ostream& result, string_view str, long)
			{ result << str; }

			static void ToStringImpl(string_ostream& result, EmptyType val, long)
			{ }

			static void ToStringImpl(string_ostream& result, NullPtrType ptr, long)
			{ result << "null"; }

			template < typename K, typename V >
			static auto ToStringImpl(string_ostream& result, const std::pair<K, V>& pair, long)
					-> decltype(ToString(result, pair.first), ToString(result, pair.second), void())
			{
				result << "[ ";
				ToString(result, pair.first);
				result << ", ";
				ToString(result, pair.second);
				result << " ]";
			}

			template < typename Types >
			static auto ToStringImpl(string_ostream& result, const Tuple<Types>& tuple, long)
					-> decltype(TestTypeListToString<Types>(result, std::make_index_sequence<GetTypeListLength<Types>::Value>()), void())
			{
				result << "[";
				For<GetTypeListLength<Types>::Value, TuplePrinter>::Do(&result, &tuple);
				result << " ]";
			}
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
	auto ToString(string_ostream& result, const T& val)
			-> decltype(Detail::TypeToStringSerializer<T>::ToStringImpl(result, val, 0), void())
	{ Detail::TypeToStringSerializer<T>::ToStringImpl(result, val, 0); }


	template < typename T >
	auto ToString(const T& val)
			-> decltype(ToString(std::declval<string_ostream&>(), val), std::string())
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
