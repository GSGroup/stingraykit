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

	template < int N, typename value_type_ = int >
	struct fixed_point
	{
		typedef value_type_ value_type;
		typedef typename IntType<sizeof(value_type) * 16, IsSigned<value_type>::Value>::ValueT DoubleType;

	private:
		value_type			_value;

		fixed_point(value_type value, bool dummy) : _value(value) { }

	public:
		inline fixed_point(value_type value = 0) : _value(value << N) { }

		template < int OtherN, typename OtherVT >
		inline fixed_point(fixed_point<OtherN, OtherVT> o)				{ assign(o); }

		inline fixed_point operator + (fixed_point o) const				{ fixed_point res(*this); return res += o; }
		inline fixed_point operator - (fixed_point o) const				{ fixed_point res(*this); return res -= o; }
		inline fixed_point operator * (fixed_point o) const				{ fixed_point res(*this); return res *= o; }
		inline fixed_point operator / (fixed_point o) const				{ fixed_point res(*this); return res /= o; }
		inline fixed_point operator - () const							{ return fixed_point(-_value, false); }

		template < typename T, typename EnableIf<IsInt<T>::Value, int>::ValueT = 0 >
		inline fixed_point operator * (T value) const					{ fixed_point res(*this); return res *= value; }

		template < typename T, typename EnableIf<IsInt<T>::Value, int>::ValueT = 0 >
		inline fixed_point operator / (T value) const					{ fixed_point res(*this); return res /= value; }

		inline fixed_point operator << (int shift) const				{ fixed_point res(*this); return res <<= shift; }
		inline fixed_point operator >> (int shift) const				{ fixed_point res(*this); return res >>= shift; }

		template < int OtherN, typename OtherVT >
		inline fixed_point& operator = (fixed_point<OtherN, OtherVT> o)	{ assign(o); return *this; }

		inline fixed_point& operator += (fixed_point o)					{ _value += o._value; return *this; }
		inline fixed_point& operator -= (fixed_point o)					{ _value -= o._value; return *this; }
		inline fixed_point& operator *= (fixed_point o)					{ _value = ((DoubleType)_value * o._value) >> N; return *this; }
		inline fixed_point& operator /= (fixed_point o)					{ _value = ((DoubleType)_value << N) / o._value; return *this; }

		template < typename T, typename EnableIf<IsInt<T>::Value, int>::ValueT = 0 >
		inline fixed_point operator *= (T value)						{ _value *= value; return *this; }

		template < typename T, typename EnableIf<IsInt<T>::Value, int>::ValueT = 0 >
		inline fixed_point& operator /= (T value)						{ _value /= value; return *this; }

		inline fixed_point& operator <<= (int shift)					{ _value <<= shift; return *this; }
		inline fixed_point& operator >>= (int shift)					{ _value >>= shift; return *this; }

		bool operator < (fixed_point o) const							{ return _value < o._value; }
		bool operator <= (fixed_point o) const							{ return _value <= o._value; }
		bool operator > (fixed_point o) const							{ return _value > o._value; }
		bool operator >= (fixed_point o) const							{ return _value >= o._value; }
		bool operator == (fixed_point o) const							{ return _value == o._value; }
		bool operator != (fixed_point o) const							{ return _value != o._value; }

		value_type to_int() const										{ return _value >> N; }
		value_type GetValue() const										{ return _value; }

		template < int OtherN, typename OtherValueType >
		void assign(fixed_point<OtherN, OtherValueType> other)
		{
			typedef typename If<(sizeof(value_type) > sizeof(OtherValueType)), value_type, OtherValueType>::ValueT BiggestType;

			if (N <= OtherN)
				_value = (BiggestType)other.GetValue() >> (OtherN - N);
			else
				_value = (BiggestType)other.GetValue() << (N - OtherN);
		}

		static fixed_point sqrt(value_type value)
		{
			fixed_point ret(0);
			DoubleType ret_sq = -((DoubleType)value << (N * 2)) - 1;
			for (int s = 30; s >= 0; s -= 2)
			{
				ret._value += ret._value;
				DoubleType b = ret_sq + ((2 * ret._value + 1) << s);
				if (b < 0)
				{
					ret_sq = b;
					++ret._value;
				}
			}
			return ret;
		}

		std::string ToString() const
		{
			value_type res = (*this * 1000).to_int();
			return StringBuilder() % (res / 1000) % "." % (res % 1000);
		}

		static fixed_point FromRawValue(value_type value)
		{ return fixed_point(value, false); }

		static fixed_point FromString(const std::string& str)
		{
			std::vector<std::string> v;
			Copy(Split(str, ".", 2), std::back_inserter(v));
			switch (v.size())
			{
			case 1:
				return stingray::FromString<value_type>(v[0]);
			case 2:
			{
				fixed_point integral_part = stingray::FromString<value_type>(v[0]);
				fixed_point fractional_part = stingray::FromString<value_type>(v[1]);
				for (size_t i = 0; i < v[1].size(); ++i)
					fractional_part /= 10;
				return integral_part + fractional_part;
			}
			default:
				STINGRAYKIT_THROW(ArgumentException("str", str));
			}
		}
	};

}

#endif
