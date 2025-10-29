#ifndef STINGRAYKIT_STRING_TOSTRING_H
#define STINGRAYKIT_STRING_TOSTRING_H

// Copyright (c) 2011 - 2025, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/collection/IEnumerable.h>
#include <stingraykit/collection/RangeBase.h>
#include <stingraykit/string/ToStringForward.h>
#include <stingraykit/optional.h>
#include <stingraykit/self_counter.h>
#include <stingraykit/variant.h>

#include <algorithm>
#include <ctype.h>
#include <limits>

namespace stingray
{

	STINGRAYKIT_DECLARE_METHOD_CHECK(FromString);

	namespace Detail
	{

		template < typename T, typename Enabler = void >
		struct SafeEvaluator;

		template < typename T >
		struct SafeEvaluator<T, typename EnableIf<std::numeric_limits<T>::is_specialized && std::numeric_limits<T>::is_integer && std::numeric_limits<T>::is_signed, void>::ValueT>
		{
		public:
			using ValueType = typename If<std::numeric_limits<T>::digits < std::numeric_limits<s8>::digits, s8, T>::ValueT;

		private:
			static constexpr ValueType MinMultiplicand = std::numeric_limits<T>::min() / 10;
			static constexpr ValueType MinMultiplicandRemainder = std::numeric_limits<T>::min() % 10 * -1;
			static constexpr ValueType MaxMultiplicand = std::numeric_limits<T>::max() / 10;
			static constexpr ValueType MaxMultiplicandRemainder = std::numeric_limits<T>::max() % 10;

			static_assert(MinMultiplicand < 0 && MinMultiplicandRemainder >= 0, "Deviant signed type detected");

		public:
			template < typename StringType >
			static auto Do(const StringType& str, ValueType value, ValueType increment, bool negative)
					-> decltype(value * 10 + (negative ? -increment : increment), ValueType())
			{
				if (negative)
					STINGRAYKIT_CHECK(value > MinMultiplicand || (value == MinMultiplicand && increment <= MinMultiplicandRemainder), IndexOutOfRangeException(str, std::numeric_limits<T>::min(), std::numeric_limits<T>::max()));
				else
					STINGRAYKIT_CHECK(value < MaxMultiplicand || (value == MaxMultiplicand && increment <= MaxMultiplicandRemainder), IndexOutOfRangeException(str, std::numeric_limits<T>::min(), std::numeric_limits<T>::max()));

				return value * 10 + (negative ? -increment : increment);
			}
		};

		template < typename T >
		struct SafeEvaluator<T, typename EnableIf<std::numeric_limits<T>::is_specialized && std::numeric_limits<T>::is_integer && !std::numeric_limits<T>::is_signed, void>::ValueT>
		{
		public:
			using ValueType = typename If<std::numeric_limits<T>::digits < std::numeric_limits<u8>::digits, u8, T>::ValueT;

		private:
			static constexpr ValueType MaxMultiplicand = std::numeric_limits<T>::max() / 10;
			static constexpr ValueType MaxMultiplicandRemainder = std::numeric_limits<T>::max() % 10;

		public:
			template < typename StringType >
			static auto Do(const StringType& str, ValueType value, ValueType increment, bool negative)
					-> decltype(value * 10 + increment, ValueType())
			{
				STINGRAYKIT_CHECK(!negative, IndexOutOfRangeException(str, std::numeric_limits<T>::min(), std::numeric_limits<T>::max()));
				STINGRAYKIT_CHECK(value < MaxMultiplicand || (value == MaxMultiplicand && increment <= MaxMultiplicandRemainder), IndexOutOfRangeException(str, std::numeric_limits<T>::min(), std::numeric_limits<T>::max()));

				return value * 10 + increment;
			}
		};

		template < typename T >
		struct TypeFromStringInterpreter
		{
		private:
			template < typename ObjectType, typename StringType, typename EnableIf<!IsSame<ObjectType, std::string>::Value && !IsSame<ObjectType, string_view>::Value, int>::ValueT = 0 >
			static auto ParseIntegralType(const StringType& str)
					-> decltype(SafeEvaluator<ObjectType>::Do(str, std::declval<typename SafeEvaluator<ObjectType>::ValueType>(), std::declval<typename SafeEvaluator<ObjectType>::ValueType>(), false), ObjectType())
			{
				using ValueType = typename SafeEvaluator<ObjectType>::ValueType;

				STINGRAYKIT_CHECK(!str.empty(), ArgumentException("str"));

				ValueType value = 0;
				bool negative = false;

				size_t index = 0;
				negative = str[0] == '-';
				if (negative || str[0] == '+')
					++index;

				for (size_t index2 = index; index2 < str.size(); ++index2)
				{
					const char ch = str[index2];
					STINGRAYKIT_CHECK(ch >= '0' && ch <= '9', FormatException(str));
				}

				for (; index < str.size(); ++index)
					value = SafeEvaluator<ObjectType>::Do(str, value, static_cast<ValueType>(str[index] - '0'), negative);

				return static_cast<ObjectType>(value);
			}

			template < typename ObjectType, typename EnableIf<IsSame<ObjectType, std::string>::Value || IsSame<ObjectType, string_view>::Value, int>::ValueT = 0 >
			static ObjectType ParseIntegralType(const std::string& str)
			{ return str; }

			template < typename ObjectType, typename EnableIf<IsSame<ObjectType, std::string>::Value, int>::ValueT = 0 >
			static ObjectType ParseIntegralType(string_view str)
			{ return str.copy(); }

			template < typename ObjectType, typename EnableIf<IsSame<ObjectType, string_view>::Value, int>::ValueT = 0 >
			static ObjectType ParseIntegralType(string_view str)
			{ return str; }

			template < typename ObjectType >
			static auto FromStringViewImplImpl(string_view str, int)
					-> decltype(ObjectType::FromString(str.copy()))
			{ return ObjectType::FromString(str.copy()); }

			template < typename ObjectType >
			static auto FromStringViewImplImpl(string_view str, long)
					-> decltype(ParseIntegralType<ObjectType>(str))
			{ return ParseIntegralType<ObjectType>(str); }

		public:
			template < typename ObjectType >
			static auto FromStringImpl(const std::string& str, int)
					-> decltype(ObjectType::FromString(str))
			{ return ObjectType::FromString(str); }

			template < typename ObjectType >
			static auto FromStringImpl(const std::string& str, long)
					-> decltype(ParseIntegralType<ObjectType>(str))
			{ return ParseIntegralType<ObjectType>(str); }

			template < typename ObjectType >
			static auto FromStringViewImpl(string_view str, int)
					-> decltype(ObjectType::FromString(str))
			{ return ObjectType::FromString(str); }

			template < typename ObjectType >
			static auto FromStringViewImpl(string_view str, long)
					-> decltype(FromStringViewImplImpl<ObjectType>(str, 0))
			{ return FromStringViewImplImpl<ObjectType>(str, 0); }
		};

	}


	template < typename T >
	auto FromString(const std::string& str)
			-> decltype(Detail::TypeFromStringInterpreter<T>::template FromStringImpl<T>(str, 0))
	{ return Detail::TypeFromStringInterpreter<T>::template FromStringImpl<T>(str, 0); }


	template < typename T >
	auto FromString(string_view str)
			-> decltype(Detail::TypeFromStringInterpreter<T>::template FromStringViewImpl<T>(str, 0))
	{ return Detail::TypeFromStringInterpreter<T>::template FromStringViewImpl<T>(str, 0); }


	template < typename T >
	auto FromString(const char* str)
			-> decltype(Detail::TypeFromStringInterpreter<T>::template FromStringViewImpl<T>(string_view(str), 0))
	{ return Detail::TypeFromStringInterpreter<T>::template FromStringViewImpl<T>(string_view(str), 0); }


	template < typename T >
	struct FromStringInterpreter
	{
		using RetType = T;

		RetType operator () (const std::string& str) const
		{ return FromString<RetType>(str); }

		RetType operator () (string_view str) const
		{ return FromString<RetType>(str); }

		RetType operator () (const char* str) const
		{ return FromString<RetType>(str); }
	};


	namespace Detail
	{

		template < typename T >
		struct TypeTryFromStringInterpreter
		{
			template < typename ObjectType >
			static auto TryFromStringViewImplImpl(string_view str, int)
					-> decltype(ObjectType::TryFromString(str.copy()))
			{ return ObjectType::TryFromString(str.copy()); }

			template < typename ObjectType >
			static auto TryFromStringViewImplImpl(string_view str, long)
					-> optional<decltype(FromString<ObjectType>(str))>
			{
				try
				{ return FromString<ObjectType>(str); }
				catch (const std::exception&)
				{ }

				return null;
			}

		public:
			template < typename ObjectType >
			static auto TryFromStringImpl(const std::string& str, int)
					-> decltype(ObjectType::TryFromString(str))
			{ return ObjectType::TryFromString(str); }

			template < typename ObjectType >
			static auto TryFromStringImpl(const std::string& str, long)
					-> optional<decltype(FromString<ObjectType>(str))>
			{
				try
				{ return FromString<ObjectType>(str); }
				catch (const std::exception&)
				{ }

				return null;
			}

			template < typename ObjectType >
			static auto TryFromStringViewImpl(string_view str, int)
					-> decltype(ObjectType::TryFromString(str))
			{ return ObjectType::TryFromString(str); }

			template < typename ObjectType >
			static auto TryFromStringViewImpl(string_view str, long)
					-> decltype(TryFromStringViewImplImpl<T>(str, 0))
			{ return TryFromStringViewImplImpl<T>(str, 0); }
		};

	}


	template < typename T >
	auto TryFromString(const std::string& str)
			-> decltype(Detail::TypeTryFromStringInterpreter<T>::template TryFromStringImpl<T>(str, 0))
	{ return Detail::TypeTryFromStringInterpreter<T>::template TryFromStringImpl<T>(str, 0); }


	template < typename T >
	auto TryFromString(string_view str)
			-> decltype(Detail::TypeTryFromStringInterpreter<T>::template TryFromStringViewImpl<T>(str, 0))
	{ return Detail::TypeTryFromStringInterpreter<T>::template TryFromStringViewImpl<T>(str, 0); }


	template < typename T >
	auto TryFromString(const char* str)
			-> decltype(Detail::TypeTryFromStringInterpreter<T>::template TryFromStringViewImpl<T>(string_view(str), 0))
	{ return Detail::TypeTryFromStringInterpreter<T>::template TryFromStringViewImpl<T>(string_view(str), 0); }


	template < typename T >
	struct TryFromStringInterpreter
	{
		using RetType = optional<T>;

		RetType operator () (const std::string& str) const
		{ return TryFromString<T>(str); }

		RetType operator () (string_view str) const
		{ return TryFromString<T>(str); }

		RetType operator () (const char* str) const
		{ return TryFromString<T>(str); }
	};


	STINGRAYKIT_DECLARE_METHOD_CHECK(ToString);


	namespace Detail
	{

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
					if (Index != 0)
						*result << ", ";

					ToString(*result, tuple->template Get<Index>());
				}
			};

			class VariantVisitor : public static_visitor<void>
			{
			private:
				string_ostream&			_result;

			public:
				explicit VariantVisitor(string_ostream& result) : _result(result) { }

				template < typename U >
				void operator () (const U& val) const { ToString(_result, val); }
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

			template < typename ItemPrinter, typename ObjectType, typename EnableIf<!IsSame<ObjectType, typename Decay<decltype(*std::declval<ObjectType>().begin())>::ValueT>::Value, int>::ValueT = 0 >
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
			static auto SpecialToString(string_ostream& result, const ObjectType& object, typename EnableIf<IsSharedPtr<ObjectType>::Value || IsSelfCountPtr<ObjectType>::Value || IsUniquePtr<ObjectType>::Value || IsOptional<ObjectType>::Value, int>::ValueT)
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
				result << "(";
				ToString(result, pair.first);
				result << ", ";
				ToString(result, pair.second);
				result << ")";
			}

			template < typename Types >
			static auto ToStringImpl(string_ostream& result, const Tuple<Types>& tuple, long)
					-> decltype(TestTypeListToString<Types>(result, std::make_index_sequence<GetTypeListLength<Types>::Value>()), void())
			{
				result << "(";
				For<GetTypeListLength<Types>::Value, TuplePrinter>::Do(&result, &tuple);
				result << ")";
			}

			template < typename Types >
			static auto ToStringImpl(string_ostream& result, const variant<Types>& var, long)
					-> decltype(TestTypeListToString<Types>(result, std::make_index_sequence<GetTypeListLength<Types>::Value>()), void())
			{ var.ApplyVisitor(VariantVisitor(result)); }
		};

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


	inline std::string ToString(const std::string& str)
	{ return str; }


	inline std::string ToString(string_view str)
	{ return str.copy(); }


	template < typename T >
	std::string ToStringPrinter::operator () (const T& val) const
	{ return ToString(val); }


	namespace Detail
	{

		template < typename T, typename StringType >
		auto TestFromStringInterpretable(int) -> decltype(FromString<T>(std::declval<StringType>()), TrueType());
		template < typename T, typename StringType >
		FalseType TestFromStringInterpretable(long);

		template < typename T >
		static auto TestStringRepresentable(int) -> decltype(ToString(std::declval<T>()), TrueType());
		template < typename T >
		static FalseType TestStringRepresentable(long);

	}


	template < typename T, typename StringType = std::string >
	struct IsFromStringInterpretable : integral_constant<bool, decltype(Detail::TestFromStringInterpretable<T, StringType>(0))::Value>
	{ };


	template < typename T >
	struct IsStringRepresentable : integral_constant<bool, decltype(Detail::TestStringRepresentable<T>(0))::Value>
	{ };


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


	template < bool EnableSeparator >
	struct SeparatorEnablerType { };


	extern const SeparatorEnablerType<true> Sep;
	extern const SeparatorEnablerType<false> NoSep;


	class StringJoiner
	{
		template < bool EnableSeparator >
		class SeparatorEnabler
		{
		private:
			StringJoiner&		_inst;

		public:
			explicit SeparatorEnabler(StringJoiner& inst)
				:	_inst(inst)
			{ }

			template < typename T >
			StringJoiner& operator % (const T& value)
			{
				_inst.Append(value, EnableSeparator);
				return _inst;
			}
		};

	private:
		StringBuilder	_builder;

		std::string		_separator;

		std::string		_prefix;
		std::string		_suffix;

		bool			_enableSeparator;

	public:
		explicit StringJoiner(const std::string& separator, bool enableSeparator = true)
			:	_separator(separator),
				_enableSeparator(enableSeparator)
		{ }

		StringJoiner(const std::string& separator, const std::string& prefix, const std::string& suffix, bool enableSeparator = true)
			:	_separator(separator),
				_prefix(prefix),
				_suffix(suffix),
				_enableSeparator(enableSeparator)
		{ }

		template < typename T >
		StringJoiner& operator % (const T& value)
		{
			Append(value, _enableSeparator);
			return *this;
		}

		template < bool EnableSeparator >
		SeparatorEnabler<EnableSeparator> operator % (SeparatorEnablerType<EnableSeparator>)
		{ return SeparatorEnabler<EnableSeparator>(*this); }

		bool empty() const { return _builder.empty(); }

		operator std::string () const { return ToString(); }

		std::string ToString() const
		{
			std::string result = _builder.empty() ? _prefix : _builder;
			result.append(_suffix);
			return result;
		}

	private:
		template < typename T >
		void Append(const T& value, bool enableSeparator)
		{
			if (_builder.empty())
				_builder % _prefix;
			else if (enableSeparator)
				_builder % _separator;

			_builder % value;
		}
	};

}

#endif
