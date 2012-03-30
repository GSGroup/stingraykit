#ifndef __GS_DVRLIB_TOOLKIT_ENUMTOSTRING_H__
#define __GS_DVRLIB_TOOLKIT_ENUMTOSTRING_H__


#include <map>
#include <vector>
#include <string>
#include <sstream>
#include <iostream>
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

		template < typename EnumClassT >
		class EnumToStringMap
		{
		private:
			typedef typename EnumClassT::Enum			NativeEnum;
			typedef std::map<NativeEnum, std::string>	EnumToStrMap;
			typedef std::map<std::string, NativeEnum>	StrToEnumMap;
			typedef std::vector<NativeEnum>				EnumValuesVec;

			EnumToStrMap	_enumToStr;
			StrToEnumMap	_strToEnum;
			EnumValuesVec	_values;

		public:
			const EnumValuesVec& GetEnumValues() const { return _values; }

			void Init(const Detail::EnumValueHolder* valuesBegin, const Detail::EnumValueHolder* valuesEnd, const char* str)
			{
				if (!Initialized())
				{
					EnumValuesVec values;
					s32 cur_value = 0;
					for (; valuesBegin != valuesEnd; ++valuesBegin, ++cur_value)
					{
						if (valuesBegin->Val != Detail::EnumValueHolder::Uninitialized)
							cur_value = valuesBegin->Val;

						values.push_back((NativeEnum)cur_value);
					}
					Instance().Init(values, str);
				}
			}

			static std::string EnumToString(NativeEnum val)
			{
				if (!Initialized())
					throw std::runtime_error("EnumToStringMap instance not initialized!");

				const EnumToStrMap& m = Instance()._enumToStr;
				typename EnumToStrMap::const_iterator it = m.find(val);
				if (it != m.end())
					return it->second;

				s32 flagged_val = 0;
				std::string result;
				for (typename EnumToStrMap::const_iterator it = m.begin(); it != m.end(); ++it)
				{
					if (((s32)it->first & val) != 0 &&
						((s32)it->first & ~val) == 0 &&
						((s32)it->first & ~flagged_val) != 0)
					{
						flagged_val |= (s32)it->first;
						if (!result.empty())
							result += "|";
						result += it->second;
					}

					if (flagged_val == val)
						break;
				}

				if (flagged_val != val)
				{
					std::stringstream s;
					s << (unsigned)val;
					return s.str();
				}

				return result;
			}

			static NativeEnum EnumFromString(const std::string& str)
			{
				if (!Initialized())
					throw std::runtime_error("EnumToStringMap instance not initialized!");

				{
					std::string::const_iterator s_it = str.begin();
					while (s_it != str.end() && IsWhitespace(*s_it))
						++s_it;

					if (s_it != str.end() && *s_it >= '0' && *s_it <= '9')
					{
						std::stringstream s(str);
						unsigned val;
						s >> val;
						if (s.fail())
							throw std::runtime_error("Cannot parse enum class value: '" + str + "'!");
						while (!s.eof())
						{
							char c;
							s >> c;
							if (!s.eof() && !IsWhitespace(c))
								throw std::runtime_error("Cannot parse enum class value: '" + str + "'!");
						}
						return (NativeEnum)val;
					}
				}

				const StrToEnumMap& m = Instance()._strToEnum;
				typename StrToEnumMap::const_iterator it = m.find(str);
				if (it != m.end())
					return it->second;

				bool has_nonwhitespace_chars = false;
				s32 result = 0;
				std::string bit_val;
				for (std::string::const_iterator s_it = str.begin(); s_it != str.end(); ++s_it)
				{
					for (; s_it != str.end() && IsWhitespace(*s_it); ++s_it);
					if (s_it != str.end())
						has_nonwhitespace_chars = true;
					for (; s_it != str.end() && !IsWhitespace(*s_it) && *s_it != '|'; ++s_it)
						bit_val += *s_it;
					for (; s_it != str.end() && IsWhitespace(*s_it); ++s_it);

					if (!bit_val.empty())
					{
						it = m.find(bit_val);
						if (it == m.end())
							throw std::runtime_error("Cannot parse enum class value: '" + str + "'!");

						bit_val.clear();
						result |= (s32)it->second;
					}

					if (s_it == str.end())
						break;
				}

				if (!has_nonwhitespace_chars)
					throw std::runtime_error("Cannot parse enum class value: '" + str + "'!");

				return (NativeEnum)result;
			}

			static bool& Initialized()
			{
				//static bool val = false;
				//return val;
				return s_instanceCreator.Initialized;
			}

			static EnumToStringMap& Instance()
			{
				static EnumToStringMap inst;
				return inst;
			}

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

			void Init(const EnumValuesVec& values, const std::string& str)
			{
				_values = values;
				std::string current_name;
				std::string::const_iterator s_it = str.begin();
				typename std::vector<NativeEnum>::const_iterator v_it = values.begin();
				for (; v_it != values.end(); ++v_it)
				{
					if (s_it == str.end())
						break;

					for (; s_it != str.end() && IsWhitespace(*s_it); ++s_it);
					for (; s_it != str.end() && !IsWhitespace(*s_it) && *s_it != ','; ++s_it)
						current_name += *s_it;
					for (; s_it != str.end() && *s_it != ','; ++s_it);

					if (s_it != str.end())
						++s_it; // ','

					_enumToStr.insert(typename EnumToStrMap::value_type(*v_it, current_name));
					_strToEnum.insert(typename StrToEnumMap::value_type(current_name, *v_it));
					current_name.clear();
				}

				if (v_it != values.end())
					throw std::runtime_error("Internal error in EnumToStringMap!");

				Initialized() = true;
			}

			static bool IsWhitespace(char c)
			{ return c == ' ' || c == '\t' || c == '\n' || c == '\r'; }
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
		private:
			typedef typename EnumClassT::Enum							NativeEnum;
			typedef typename std::vector<NativeEnum>::const_iterator	Wrapped;
			friend struct EnumIteratorCreator<EnumClassT>;
		private:
			Wrapped				_wrapped;
			mutable EnumClassT	_value;
			EnumIterator(const Wrapped& wrapped) : _wrapped(wrapped) { }
		public:
			reference dereference() const { _value = *_wrapped; return _value; }
			void increment() { ++_wrapped; }
			void decrement() { --_wrapped; }
			void advance(const difference_type& diff) { std::advance(_wrapped, diff); }
			difference_type distance_to(const EnumIterator &other) const { return std::distance(_wrapped, other._wrapped); }
			bool equal(const EnumIterator& other) const { return _wrapped == other._wrapped; }
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
