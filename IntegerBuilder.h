#ifndef STINGRAY_TOOLKIT_INTEGERBUILDER_H
#define STINGRAY_TOOLKIT_INTEGERBUILDER_H


#include <stingray/toolkit/Types.h>


namespace stingray
{


    template<typename IntegerType>
    class IntegerBuilder
    {
    private:
		IntegerType _result;

    public:
        IntegerBuilder()
        	: _result(0)
        { CompileTimeAssert<IsFixedWidthIntType<IntegerType>::Value>(); }

        template<typename U>
        IntegerBuilder& Append(const U& field, size_t fieldSize)
        {
        	CompileTimeAssert<IsFixedWidthIntType<U>::Value>();

            _result <<= fieldSize;
            _result |= ResetMostSignificantBits(field, 8 * sizeof(U) - fieldSize);

            return *this;
        }

        IntegerBuilder& Append(bool bit)
        { return Append(static_cast<u8>(bit), 1); }

        IntegerType GetResult()
        { return _result; }

    private:
        template<typename U>
        U ResetMostSignificantBits(const U& field, size_t count)
        { return field & (static_cast<U>(~0) >> count); }
    };


}

#endif
