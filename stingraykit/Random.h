#ifndef STINGRAYKIT_RANDOM_H
#define STINGRAYKIT_RANDOM_H


#include <stingraykit/Types.h>


namespace stingray
{

	class Random
	{
	private:
		u32	_seed;

	public:
		Random();
		Random(u32 seed);

		u32 Next();
		u32 Next(u32 maxValue) { return Next() % maxValue; }
		u32 Next(u32 minValue, u32 maxValue) { return minValue + Next() % (maxValue - minValue); }
	};

}


#endif
