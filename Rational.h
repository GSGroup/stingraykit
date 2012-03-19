#ifndef __TOOLKIT_RATIONAL_H__
#define	__TOOLKIT_RATIONAL_H__

namespace stingray
{
	struct Rational
	{
		int Num, Denum;
		Rational(int Num = 0, int Denum = 1): Num(Num), Denum(Denum) {}
	};
}

#endif	/* RATIONAL_H */

