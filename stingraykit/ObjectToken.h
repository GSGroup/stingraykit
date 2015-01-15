#ifndef STINGRAYKIT_OBJECTTOKEN_H
#define STINGRAYKIT_OBJECTTOKEN_H


#include <stingraykit/IToken.h>


namespace stingray
{

	namespace Detail
	{
		template <typename T>
		struct ObjectToken : public virtual IToken
		{
		private:
			T	_object;

		public:
			ObjectToken(const T& object) : _object(object)
			{ }
		};
	}


	template <typename T>
	Token MakeObjectToken(const T& object)
	{ return MakeToken<Detail::ObjectToken<T> >(object); }

}

#endif
