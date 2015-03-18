#ifndef STINGRAYKIT_STRING_LEXICAL_CAST_H
#define STINGRAYKIT_STRING_LEXICAL_CAST_H

#include <stingraykit/string/ToString.h>

namespace stingray
{

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

}

#endif
