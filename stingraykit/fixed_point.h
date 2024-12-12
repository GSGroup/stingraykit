#ifndef STINGRAYKIT_FIXED_POINT_H
#define STINGRAYKIT_FIXED_POINT_H

// Copyright (c) 2011 - 2024, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/string/ToString.h>

namespace stingray
{

	template < int N, typename ValueType_ = int >
	class fixed_point
	{
		using ValueType = ValueType_;
		using DoubleType = typename IntType<sizeof(ValueType) * 16, IsSigned<ValueType>::Value>::ValueT;

	private:
		ValueType			_value;

	public:
		constexpr fixed_point(ValueType value = 0) : _value(value << N) { }

		template < int OtherN, typename OtherVT >
		constexpr fixed_point(fixed_point<OtherN, OtherVT> other)					{ assign(other); }

		constexpr fixed_point& operator += (fixed_point other)						{ _value += other._value; return *this; }
		constexpr fixed_point operator + (fixed_point other) const					{ fixed_point result(*this); return result += other; }

		constexpr fixed_point& operator -= (fixed_point other)						{ _value -= other._value; return *this; }
		constexpr fixed_point operator - (fixed_point other) const					{ fixed_point result(*this); return result -= other; }
		constexpr fixed_point operator - () const									{ return fixed_point(-_value, false); }

		constexpr fixed_point& operator *= (fixed_point other)						{ _value = ((DoubleType)_value * other._value) >> N; return *this; }
		constexpr fixed_point operator * (fixed_point other) const					{ fixed_point result(*this); return result *= other; }

		template < typename T, typename EnableIf<IsInt<T>::Value, int>::ValueT = 0 >
		constexpr fixed_point operator *= (T value)									{ _value *= value; return *this; }
		template < typename T, typename EnableIf<IsInt<T>::Value, int>::ValueT = 0 >
		constexpr fixed_point operator * (T value) const							{ fixed_point result(*this); return result *= value; }

		constexpr fixed_point& operator /= (fixed_point other)						{ _value = ((DoubleType)_value << N) / other._value; return *this; }
		constexpr fixed_point operator / (fixed_point other) const					{ fixed_point result(*this); return result /= other; }

		template < typename T, typename EnableIf<IsInt<T>::Value, int>::ValueT = 0 >
		constexpr fixed_point& operator /= (T value)								{ _value /= value; return *this; }
		template < typename T, typename EnableIf<IsInt<T>::Value, int>::ValueT = 0 >
		constexpr fixed_point operator / (T value) const							{ fixed_point result(*this); return result /= value; }

		constexpr fixed_point& operator <<= (int shift)								{ _value <<= shift; return *this; }
		constexpr fixed_point operator << (int shift) const							{ fixed_point result(*this); return result <<= shift; }

		constexpr fixed_point& operator >>= (int shift)								{ _value >>= shift; return *this; }
		constexpr fixed_point operator >> (int shift) const							{ fixed_point result(*this); return result >>= shift; }

		template < int OtherN, typename OtherVT >
		constexpr fixed_point& operator = (fixed_point<OtherN, OtherVT> other)		{ assign(other); return *this; }

		constexpr bool operator < (fixed_point other) const							{ return _value < other._value; }
		STINGRAYKIT_GENERATE_CONSTEXPR_COMPARISON_OPERATORS_FROM_LESS(fixed_point);

		template < int OtherN, typename OtherValueType >
		constexpr void assign(fixed_point<OtherN, OtherValueType> other)
		{
			using BiggestType = typename If<(sizeof(ValueType) > sizeof(OtherValueType)), ValueType, OtherValueType>::ValueT;

			if (N <= OtherN)
				_value = (BiggestType)other.GetValue() >> (OtherN - N);
			else
				_value = (BiggestType)other.GetValue() << (N - OtherN);
		}

		constexpr static fixed_point sqrt(ValueType value)
		{
			fixed_point result(0);
			DoubleType resultSq = -((DoubleType)value << (N * 2)) - 1;

			for (int s = 30; s >= 0; s -= 2)
			{
				result._value += result._value;

				const DoubleType b = resultSq + ((2 * result._value + 1) << s);
				if (b < 0)
				{
					resultSq = b;
					++result._value;
				}
			}

			return result;
		}

		constexpr ValueType to_int() const									{ return _value >> N; }

		constexpr ValueType GetValue() const									{ return _value; }
		constexpr static fixed_point FromRawValue(ValueType value)			{ return fixed_point(value, false); }

		std::string ToString() const
		{
			const ValueType result = (*this * 1000).to_int();
			const std::string fractionalPart = stingray::ToString(result % 1000);
			return StringBuilder() % (result / 1000) % "." % std::string(3 - fractionalPart.size(), '0') % fractionalPart;
		}

		static fixed_point FromString(const std::string& str)
		{
			STINGRAYKIT_CHECK(!str.empty(), ArgumentException("str"));

			const size_t pointPos = str.find('.');
			if (pointPos == std::string::npos)
				return stingray::FromString<ValueType>(str);

			const fixed_point integralPart = stingray::FromString<ValueType>(str.substr(0, pointPos));
			const std::string rawFractionalPart = str.substr(pointPos + 1);

			fixed_point fractionalPart = stingray::FromString<ValueType>(rawFractionalPart);
			for (size_t i = 0; i < rawFractionalPart.size(); ++i)
				fractionalPart /= 10;

			return integralPart + fractionalPart;
		}

	private:
		constexpr fixed_point(ValueType value, bool dummy) : _value(value) { }
	};

}

#endif
