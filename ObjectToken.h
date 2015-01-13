#ifndef STINGRAYKIT_OBJECTTOKEN_H
#define STINGRAYKIT_OBJECTTOKEN_H


#include <stingray/toolkit/IToken.h>


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
	ITokenPtr MakeObjectToken(const T& object)
	{ return make_shared<Detail::ObjectToken<T> >(object); }

}

#endif
