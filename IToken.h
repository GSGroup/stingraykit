#ifndef STINGRAY_TOOLKIT_ITOKEN_H
#define STINGRAY_TOOLKIT_ITOKEN_H


namespace stingray
{

	struct IToken
	{
		virtual ~IToken() { }
	};
	TOOLKIT_DECLARE_PTR(IToken);

}


#endif
