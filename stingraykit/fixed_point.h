#ifndef STINGRAYKIT_FIXED_POINT_H
#define STINGRAYKIT_FIXED_POINT_H

// Copyright (c) 2011 - 2024, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/string/StringUtils.h>
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
		fixed_point(ValueType value = 0) : _value(value << N) { }

		template < int OtherN, typename OtherVT >
		fixed_point(fixed_point<OtherN, OtherVT> other)					{ assign(other); }

		fixed_point& operator += (fixed_point other)					{ _value += other._value; return *this; }
		fixed_point operator + (fixed_point other) const				{ fixed_point result(*this); return result += other; }

		fixed_point& operator -= (fixed_point other)					{ _value -= other._value; return *this; }
		fixed_point operator - (fixed_point other) const				{ fixed_point result(*this); return result -= other; }
		fixed_point operator - () const									{ return fixed_point(-_value, false); }

		fixed_point& operator *= (fixed_point other)					{ _value = ((DoubleType)_value * other._value) >> N; return *this; }
		fixed_point operator * (fixed_point other) const				{ fixed_point result(*this); return result *= other; }

		template < typename T, typename EnableIf<IsInt<T>::Value, int>::ValueT = 0 >
		fixed_point operator *= (T value)								{ _value *= value; return *this; }
		template < typename T, typename EnableIf<IsInt<T>::Value, int>::ValueT = 0 >
		fixed_point operator * (T value) const							{ fixed_point result(*this); return result *= value; }

		fixed_point& operator /= (fixed_point other)					{ _value = ((DoubleType)_value << N) / other._value; return *this; }
		fixed_point operator / (fixed_point other) const				{ fixed_point result(*this); return result /= other; }

		template < typename T, typename EnableIf<IsInt<T>::Value, int>::ValueT = 0 >
		fixed_point& operator /= (T value)								{ _value /= value; return *this; }
		template < typename T, typename EnableIf<IsInt<T>::Value, int>::ValueT = 0 >
		fixed_point operator / (T value) const							{ fixed_point result(*this); return result /= value; }

		fixed_point& operator <<= (int shift)							{ _value <<= shift; return *this; }
		fixed_point operator << (int shift) const						{ fixed_point result(*this); return result <<= shift; }

		fixed_point& operator >>= (int shift)							{ _value >>= shift; return *this; }
		fixed_point operator >> (int shift) const						{ fixed_point result(*this); return result >>= shift; }

		template < int OtherN, typename OtherVT >
		fixed_point& operator = (fixed_point<OtherN, OtherVT> other)	{ assign(other); return *this; }

		bool operator < (fixed_point other) const						{ return _value < other._value; }
		STINGRAYKIT_GENERATE_COMPARISON_OPERATORS_FROM_LESS(fixed_point);

		template < int OtherN, typename OtherValueType >
		void assign(fixed_point<OtherN, OtherValueType> other)
		{
			using BiggestType = typename If<(sizeof(ValueType) > sizeof(OtherValueType)), ValueType, OtherValueType>::ValueT;

			if (N <= OtherN)
				_value = (BiggestType)other.GetValue() >> (OtherN - N);
			else
				_value = (BiggestType)other.GetValue() << (N - OtherN);
		}

		static fixed_point sqrt(ValueType value)
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

		ValueType to_int() const									{ return _value >> N; }

		ValueType GetValue() const									{ return _value; }
		static fixed_point FromRawValue(ValueType value)			{ return fixed_point(value, false); }

		std::string ToString() const
		{
			const ValueType result = (*this * 1000).to_int();
			return StringBuilder() % (result / 1000) % "." % (result % 1000);
		}

		static fixed_point FromString(const std::string& str)
		{
			std::vector<std::string> v;
			Copy(Split(str, ".", 2), std::back_inserter(v));

			switch (v.size())
			{
			case 1:
				return stingray::FromString<ValueType>(v[0]);
			case 2:
			{
				const fixed_point integralPart = stingray::FromString<ValueType>(v[0]);

				fixed_point fractionalPart = stingray::FromString<ValueType>(v[1]);
				for (size_t i = 0; i < v[1].size(); ++i)
					fractionalPart /= 10;

				return integralPart + fractionalPart;
			}
			default:
				STINGRAYKIT_THROW(ArgumentException("str", str));
			}
		}

	private:
		fixed_point(ValueType value, bool dummy) : _value(value) { }
	};

}

#endif
