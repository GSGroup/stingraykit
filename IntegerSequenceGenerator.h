#ifndef STINGRAY_TOOLKIT_INTEGERSEQUENCEGENERATOR_H
#define STINGRAY_TOOLKIT_INTEGERSEQUENCEGENERATOR_H


#include <stingray/toolkit/thread/atomic.h>


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
