#include <stingraykit/Random.h>

#include <ctime>


namespace stingray
{

	Random::Random()
		: _seed(std::time(0) & 0x7fffffffU)
	{ }


	Random::Random(u32 seed)
		: _seed(seed & 0x7fffffffU)
	{ }


	u32 Random::Next()
	{
		_seed = (_seed * 1103515245U + 12345U) & 0x7fffffffU;
		return _seed;
	}

}
