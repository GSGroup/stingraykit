#ifndef STINGRAYKIT_ENUM_H
#define STINGRAYKIT_ENUM_H

// Copyright (c) 2011 - 2024, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/PhoenixSingleton.h>
#include <stingraykit/collection/iterator_base.h>

#include <vector>
#include <string>

#define STINGRAYKIT_ENUM_VALUES(...) \
	private: \
		static void InitEnumToStringMap(::stingray::Detail::EnumToStringMapBase& map) \
		{ \
			stingray::Detail::EnumValueHolder __VA_ARGS__; \
			stingray::Detail::EnumValueHolder values[] = { __VA_ARGS__ }; \
			map.Init(values, values + sizeof(values) / sizeof(values[0]), #__VA_ARGS__); \
		} \
	public: \
		enum Enum { __VA_ARGS__ }

#define STINGRAYKIT_DECLARE_ENUM_CLASS(ClassName) \
		friend class stingray::Detail::EnumToStringMapInstance<ClassName>; \
	public: \
		using const_iterator = stingray::Detail::EnumIterator<ClassName>; \
		ClassName() : _enumVal() \
		{ \
			const std::vector<int>& values = stingray::Detail::EnumToStringMap<ClassName>::GetEnumValues(); \
			if (!values.empty()) \
				_enumVal = static_cast<Enum>(values.front()); \
		} \
		constexpr ClassName(Enum enumVal) : _enumVal(enumVal) { } \
		static const_iterator begin()						{ return stingray::Detail::EnumIteratorCreator<ClassName>::begin(); } \
		static const_iterator end()							{ return stingray::Detail::EnumIteratorCreator<ClassName>::end(); } \
		std::string ToString() const						{ return stingray::Detail::EnumToStringMap<ClassName>::EnumToString(_enumVal); } \
		static ClassName FromString(const std::string& str)	{ return stingray::Detail::EnumToStringMap<ClassName>::EnumFromString(str); } \
		constexpr operator Enum () const { return _enumVal; } \
		constexpr Enum val() const { return _enumVal; } \
		constexpr bool operator == (Enum other) const { return _enumVal == other; } \
		constexpr bool operator < (Enum other) const { return _enumVal < other; } \
		constexpr bool operator > (Enum other) const { return _enumVal > other; } \
		DETAIL_ENUM_CLASS_DEFINE_OPERATOR(ClassName, ==) \
		template < typename T > \
		constexpr bool operator != (T other) const { return !(*this == other); } \
		DETAIL_ENUM_CLASS_DEFINE_OPERATOR(ClassName, <) \
		DETAIL_ENUM_CLASS_DEFINE_OPERATOR(ClassName, >) \
		template < typename T > \
		constexpr bool operator <= (T other) const { return !(*this > other); } \
		template < typename T > \
		constexpr bool operator >= (T other) const { return !(*this < other); } \
	private: \
		Enum _enumVal

#define DETAIL_ENUM_CLASS_DEFINE_OPERATOR(ClassName, OP) \
		template < typename T > \
		bool operator OP (T other) const \
		{ \
			static_assert(stingray::IsInherited<T, ClassName>::Value, "Invalid enum used"); \
			return *this OP (Enum)other; \
		}


#define STINGRAYKIT_DECLARE_ENUM_CLASS_BIT_OPERATORS(ClassName_) \
		constexpr ClassName_& operator |= (ClassName_& l, ClassName_::Enum r) \
		{ return l = ClassName_(static_cast<ClassName_::Enum>(static_cast<int>(l) | static_cast<int>(r))); } \
		constexpr ClassName_& operator &= (ClassName_& l, ClassName_::Enum r) \
		{ return l = ClassName_(static_cast<ClassName_::Enum>(static_cast<int>(l) & static_cast<int>(r))); } \
		constexpr ClassName_::Enum operator | (ClassName_::Enum l, ClassName_::Enum r) \
		{ ClassName_ result(l); return result |= r; } \
		constexpr ClassName_::Enum operator & (ClassName_::Enum l, ClassName_::Enum r) \
		{ ClassName_ result(l); return result &= r; }

#define STINGRAYKIT_DECLARE_ENUM_CLASS_MEMBER_BIT_OPERATORS(ClassName_) \
	public: \
		constexpr ClassName_& operator |= (ClassName_::Enum r) \
		{ return *this = ClassName_(static_cast<ClassName_::Enum>(static_cast<int>(val()) | static_cast<int>(r))); } \
		constexpr ClassName_& operator &= (ClassName_::Enum r) \
		{ return *this = ClassName_(static_cast<ClassName_::Enum>(static_cast<int>(val()) & static_cast<int>(r))); } \
		constexpr ClassName_ operator | (ClassName_::Enum r) \
		{ ClassName_ result(*this); return result |= r; } \
		constexpr ClassName_ operator & (ClassName_::Enum r) \
		{ ClassName_ result(*this); return result &= r; }

namespace stingray
{

	namespace Detail
	{

		struct EnumValueHolder
		{
			static const s64 Uninitialized = ((s64)0x7FFFFFFF) << 32;

			s64					Value;

			EnumValueHolder() : Value(Uninitialized) { }
			EnumValueHolder(s32 value) : Value(value) { }

			EnumValueHolder& operator = (s32 value)
			{
				Value = value;
				return *this;
			}
		};

		inline EnumValueHolder operator + (const EnumValueHolder& left, const EnumValueHolder& right)
		{ return EnumValueHolder(left.Value + right.Value); }

		inline EnumValueHolder operator - (const EnumValueHolder& left, const EnumValueHolder& right)
		{ return EnumValueHolder(left.Value - right.Value); }

		class EnumToStringMapBase
		{
			STINGRAYKIT_NONCOPYABLE(EnumToStringMapBase);

		public:
			using EnumValuesVec = std::vector<int>;

		private:
			struct Impl;

		private:
			Impl*				_impl;

		public:
			const EnumValuesVec& GetEnumValues() const;

			std::string EnumToString(int value);
			int EnumFromString(const std::string& str);

			void Init(const EnumValueHolder* valuesBegin, const EnumValueHolder* valuesEnd, const char* str);

		protected:
			EnumToStringMapBase();
			~EnumToStringMapBase();
		};

		template < typename EnumClassT >
		class EnumToStringMapInstance : public EnumToStringMapBase, public PhoenixSingleton<EnumToStringMapInstance<EnumClassT>>
		{
		public:
			EnumToStringMapInstance()
			{ EnumClassT::InitEnumToStringMap(*this); }
		};

		template < typename EnumClassT >
		class EnumToStringMap
		{
			using NativeEnum = typename EnumClassT::Enum;
			using PhoenixType = EnumToStringMapInstance<EnumClassT>;
			using EnumValuesVec = EnumToStringMapBase::EnumValuesVec;

		public:
			static const EnumValuesVec& GetEnumValues()					{ return PhoenixType::Instance().GetEnumValues(); }
			static std::string EnumToString(NativeEnum value)			{ return PhoenixType::Instance().EnumToString(static_cast<int>(value)); }
			static NativeEnum EnumFromString(const std::string& str)	{ return (NativeEnum)PhoenixType::Instance().EnumFromString(str); }
		};

		template < typename EnumClassT >
		struct EnumIteratorCreator;

		template < typename EnumClassT >
		class EnumIterator : public iterator_base<EnumIterator<EnumClassT>, EnumClassT, std::random_access_iterator_tag, std::ptrdiff_t, const EnumClassT*, const EnumClassT&>
		{
			using base = iterator_base<EnumIterator<EnumClassT>, EnumClassT, std::random_access_iterator_tag, std::ptrdiff_t, const EnumClassT*, const EnumClassT&>;

			friend struct EnumIteratorCreator<EnumClassT>;

		public:
			using difference_type = typename base::difference_type;
			using reference = typename base::reference;
			using pointer = typename base::pointer;

		private:
			using NativeEnum = typename EnumClassT::Enum;
			using Wrapped = typename std::vector<int>::const_iterator;

		private:
			Wrapped				_wrapped;

		public:
			reference dereference() const									{ return *reinterpret_cast<pointer>(&*_wrapped); }

			void increment()												{ ++_wrapped; }
			void decrement()												{ --_wrapped; }
			void advance(const difference_type& diff)						{ std::advance(_wrapped, diff); }

			difference_type distance_to(const EnumIterator& other) const	{ return std::distance(_wrapped, other._wrapped); }
			bool equal(const EnumIterator& other) const						{ return _wrapped == other._wrapped; }

		private:
			EnumIterator(const Wrapped& wrapped) : _wrapped(wrapped) { }
		};

		template < typename EnumClassT >
		struct EnumIteratorCreator
		{
			static EnumIterator<EnumClassT> begin()		{ return EnumToStringMap<EnumClassT>::GetEnumValues().begin(); }
			static EnumIterator<EnumClassT> end()		{ return EnumToStringMap<EnumClassT>::GetEnumValues().end(); }
		};

	}

}

#endif
