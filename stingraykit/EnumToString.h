#ifndef STINGRAYKIT_ENUMTOSTRING_H
#define STINGRAYKIT_ENUMTOSTRING_H

// Copyright (c) 2011 - 2015, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <memory>
#include <vector>
#include <string>

#include <stingraykit/PhoenixSingleton.h>
#include <stingraykit/Types.h>
#include <stingraykit/collection/iterator_base.h>

namespace stingray
{

	namespace Detail
	{
		struct EnumValueHolder
		{
			static const s64 Uninitialized = ((s64)0x7FFFFFFF) << 32;
			s64 Val;
			EnumValueHolder() : Val(Uninitialized) { }
			EnumValueHolder(s32 val) : Val(val) { }
			EnumValueHolder& operator = (s32 val) { Val = val; return *this; }
		};
		inline EnumValueHolder operator + (EnumValueHolder const &left, EnumValueHolder const &right) { return EnumValueHolder(left.Val + right.Val); }
		inline EnumValueHolder operator - (EnumValueHolder const &left, EnumValueHolder const &right) { return EnumValueHolder(left.Val - right.Val); }

		class EnumToStringMapBase
		{
		public:
			typedef std::vector<int>	EnumValuesVec;

		private:
			struct Impl;
			std::auto_ptr<Impl>	_impl;

			EnumToStringMapBase(const EnumToStringMapBase&);
			EnumToStringMapBase& operator = (const EnumToStringMapBase&);

		protected:
			EnumToStringMapBase();
			~EnumToStringMapBase();

		public:
			const EnumValuesVec& DoGetEnumValues() const;
			std::string DoEnumToString(int val);
			int DoEnumFromString(const std::string& str);

			void DoInit(const Detail::EnumValueHolder* valuesBegin, const Detail::EnumValueHolder* valuesEnd, const char* str);
		};


		template < typename EnumClassT >
		class EnumToStringMapInstance : public EnumToStringMapBase, public PhoenixSingleton<EnumToStringMapInstance<EnumClassT> >
		{
		public:
			EnumToStringMapInstance()
			{ EnumClassT::InitEnumToStringMap(*this); }
		};


		template < typename EnumClassT >
		class EnumToStringMap
		{
			typedef typename EnumClassT::Enum NativeEnum;
			typedef EnumToStringMapInstance<EnumClassT> PhoenixType;
			typedef EnumToStringMapBase::EnumValuesVec EnumValuesVec;

		public:
			static const EnumValuesVec& GetEnumValues()					{ return PhoenixType::Instance().DoGetEnumValues(); }
			static std::string EnumToString(NativeEnum val)				{ return PhoenixType::Instance().DoEnumToString((int)val); }
			static NativeEnum EnumFromString(const std::string& str)	{ return (NativeEnum)PhoenixType::Instance().DoEnumFromString(str); }

		private:
			typedef Detail::EnumToStringMap<EnumClassT>	ThisT;
		};

		template < typename EnumClassT >
		struct EnumIteratorCreator;

		template < typename EnumClassT >
		class EnumIterator : public iterator_base<EnumIterator<EnumClassT>, EnumClassT, std::random_access_iterator_tag, std::ptrdiff_t, const EnumClassT*, const EnumClassT&>
		{
			typedef iterator_base<EnumIterator<EnumClassT>, EnumClassT, std::random_access_iterator_tag, std::ptrdiff_t, const EnumClassT*, const EnumClassT&> base;

		public:
			typedef typename base::difference_type		difference_type;
			typedef	typename base::reference			reference;
			typedef	typename base::pointer				pointer;

		private:
			typedef typename EnumClassT::Enum							NativeEnum;
			typedef typename std::vector<int>::const_iterator			Wrapped;
			friend struct EnumIteratorCreator<EnumClassT>;

		private:
			Wrapped				_wrapped;
			EnumIterator(const Wrapped& wrapped) : _wrapped(wrapped) { }

		public:
			reference dereference() const {
				return *reinterpret_cast<pointer>(&*_wrapped);
			}
			void increment()												{ ++_wrapped; }
			void decrement()												{ --_wrapped; }
			void advance(const difference_type& diff)						{ std::advance(_wrapped, diff); }
			difference_type distance_to(const EnumIterator &other) const	{ return std::distance(_wrapped, other._wrapped); }
			bool equal(const EnumIterator& other) const						{ return _wrapped == other._wrapped; }
		};

		template < typename EnumClassT >
		struct EnumIteratorCreator
		{
			static EnumIterator<EnumClassT> begin()	{ return EnumToStringMap<EnumClassT>::GetEnumValues().begin(); }
			static EnumIterator<EnumClassT> end()	{ return EnumToStringMap<EnumClassT>::GetEnumValues().end(); }
		};
	}

}


#endif
