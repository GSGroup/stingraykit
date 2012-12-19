#ifndef STINGRAY_TOOLKIT_ENUMTOSTRING_H
#define STINGRAY_TOOLKIT_ENUMTOSTRING_H

#include <map>
#include <vector>
#include <string>
#include <sstream>
#include <stdexcept>

#include <stingray/toolkit/iterator_base.h>
#include <stingray/toolkit/Types.h>

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

		void EnumToStringMap_throw(const std::string& msg);

		struct EnumToStringMapImpl
		{
			typedef std::map<int, std::string>			EnumToStrMap;
			typedef std::map<std::string, int>			StrToEnumMap;
			typedef std::vector<int>					EnumValuesVec;

			static std::string EnumToString(const EnumToStrMap& map, int val);
			static int EnumFromString(const StrToEnumMap& map, const std::string& str);
			static void Init(const EnumValuesVec& values, EnumToStrMap& enumToStr, StrToEnumMap& strToEnum, const std::string& str);

			static bool IsWhitespace(char c)
			{ return c == ' ' || c == '\t' || c == '\n' || c == '\r'; }
		};


		class EnumToStringMapBase
		{
		private:
			typedef std::map<int, std::string>			EnumToStrMap;
			typedef std::map<std::string, int>			StrToEnumMap;
			typedef std::vector<int>					EnumValuesVec;

			EnumToStrMap	_enumToStr;
			StrToEnumMap	_strToEnum;
			EnumValuesVec	_values;

		public:
			const EnumValuesVec& GetEnumValues() const { return _values; }

			void DoInit(const Detail::EnumValueHolder* valuesBegin, const Detail::EnumValueHolder* valuesEnd, const char* str, bool& initialized);

			std::string DoEnumToString(int val)
			{ return EnumToStringMapImpl::EnumToString(_enumToStr, (int)val); }
			int DoEnumFromString(const std::string& str)
			{ return EnumToStringMapImpl::EnumFromString(_strToEnum, str); }
		};


		template < typename EnumClassT >
		class EnumToStringMap : public EnumToStringMapBase
		{
			typedef typename EnumClassT::Enum NativeEnum;

		public:
			static std::string EnumToString(NativeEnum val)
			{
				if (!Initialized())
					EnumToStringMap_throw("EnumToStringMap instance not initialized!");

				return Instance().DoEnumToString((int)val);
			}

			static NativeEnum EnumFromString(const std::string& str)
			{
				if (!Initialized())
					EnumToStringMap_throw("EnumToStringMap instance not initialized!");

				return (NativeEnum)Instance().DoEnumFromString(str);
			}

			static bool& Initialized()
			{ return s_instanceCreator.Initialized; }

			static EnumToStringMap& Instance()
			{
				static EnumToStringMap inst;
				return inst;
			}

			void Init(const Detail::EnumValueHolder* valuesBegin, const Detail::EnumValueHolder* valuesEnd, const char* str)
			{ DoInit(valuesBegin, valuesEnd, str, Initialized()); }

		private:
			struct InstanceCreator
			{
				bool Initialized;

				InstanceCreator() : Initialized(false) { EnumToStringMap::CallInitEnumToStringMap(); }
			};
			static InstanceCreator s_instanceCreator;

			static void CallInitEnumToStringMap()
			{ EnumClassT::InitEnumToStringMap((EnumClassT*)0); }


			EnumToStringMap() { }
			EnumToStringMap(const EnumToStringMap&);
			EnumToStringMap& operator = (const EnumToStringMap&);
		};

		template < typename EnumClassT >
		typename EnumToStringMap<EnumClassT>::InstanceCreator EnumToStringMap<EnumClassT>::s_instanceCreator;

		template < typename EnumClassT >
		inline EnumToStringMap<EnumClassT>& GetEnumToStringMap(const EnumClassT*)
		{ return EnumToStringMap<EnumClassT>::Instance(); }

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
			static EnumIterator<EnumClassT> begin()	{ return GetEnumToStringMap<EnumClassT>(NULL).GetEnumValues().begin(); }
			static EnumIterator<EnumClassT> end()	{ return GetEnumToStringMap<EnumClassT>(NULL).GetEnumValues().end(); }
		};
	}

}


#endif
