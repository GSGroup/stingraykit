#ifndef STINGRAYKIT_INTEGERSEQUENCEGENERATOR_H
#define STINGRAYKIT_INTEGERSEQUENCEGENERATOR_H


#include <stingraykit/thread/atomic.h>


namespace stingray
{

	template < typename IntegerType >
	class IntegerSequenceGenerator
	{
	private:
		atomic<IntegerType>		_sequence;

	public:
		explicit IntegerSequenceGenerator(IntegerType sequenceStart = IntegerType())
			: _sequence(sequenceStart)
		{ }

		IntegerType Next() { return _sequence++; }
	};

}


#endif
