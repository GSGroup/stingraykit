#ifndef STINGRAYKIT_FIXED_POINT_H
#define STINGRAYKIT_FIXED_POINT_H


#include <stingraykit/string/StringUtils.h>

namespace stingray
{


	template<int N, typename value_type_ = int>
	struct fixed_point
	{
		typedef value_type_ value_type;

	private:
		value_type _value;

	public:
		inline fixed_point(value_type value = 0) : _value(value << N) {}

		template<int OtherN, typename OtherVT>
		inline fixed_point(fixed_point<OtherN, OtherVT> o)	{ assign(o); }

		inline fixed_point operator+ (fixed_point o) const	{ fixed_point res(*this); return res += o; }
		inline fixed_point operator- (fixed_point o) const	{ fixed_point res(*this); return res -= o; }
		inline fixed_point operator* (fixed_point o) const	{ fixed_point res(*this); return res *= o; }
		inline fixed_point operator/ (fixed_point o) const	{ fixed_point res(*this); return res /= o; }

		template<typename T>
		inline typename EnableIf<IsIntType<T>::Value, fixed_point>::ValueT operator* (T value) const
		{ fixed_point res(*this); return res *= value; }

		template<typename T>
		inline typename EnableIf<IsIntType<T>::Value, fixed_point>::ValueT operator/ (T value) const
		{ fixed_point res(*this); return res /= value; }

		inline fixed_point operator<<(int shift) const		{ fixed_point res(*this); return res <<= shift; }
		inline fixed_point operator>>(int shift) const		{ fixed_point res(*this); return res >>= shift; }


		template<int OtherN, typename OtherVT>
		inline fixed_point& operator= (fixed_point<OtherN, OtherVT> o)	{ assign(o); return *this; }

		inline fixed_point& operator+= (fixed_point o)					{ _value += o._value; return *this; }
		inline fixed_point& operator-= (fixed_point o)					{ _value -= o._value; return *this; }
		inline fixed_point& operator*= (fixed_point o)					{ _value = (_value * o._value) >> N; return *this; }
		inline fixed_point& operator/= (fixed_point o)					{ _value = (_value << N) / o._value; return *this; }


		template<typename T>
		inline typename EnableIf<IsIntType<T>::Value, fixed_point&>::ValueT operator*= (T value)
		{ _value *= value; return *this; }

		template<typename T>
		inline typename EnableIf<IsIntType<T>::Value, fixed_point&>::ValueT operator/= (T value)
		{ _value /= value; return *this; }

		inline fixed_point& operator<<=(int shift)			{ _value <<= shift; return *this; }
		inline fixed_point& operator>>=(int shift)			{ _value >>= shift; return *this; }

		bool operator< (fixed_point o) const	{ return _value < o._value; }
		bool operator<=(fixed_point o) const	{ return _value <= o._value; }
		bool operator> (fixed_point o) const	{ return _value > o._value; }
		bool operator>=(fixed_point o) const	{ return _value >= o._value; }
		bool operator==(fixed_point o) const	{ return _value == o._value; }
		bool operator!=(fixed_point o) const	{ return _value != o._value; }

		value_type to_int() const				{ return _value >> N; }
		value_type GetValue() const				{ return _value; }

		template<int OtherN, typename OtherValueType>
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
			value_type ret_sq = -(value << (N * 2)) - 1;
			for (int s = 30; s >= 0; s -= 2)
			{
				ret._value += ret._value;
				value_type b = ret_sq + ((2 * ret._value + 1) << s);
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

		static fixed_point FromString(const std::string& str)
		{
			std::vector<std::string> v;
			Split(str, ".", v);
			STINGRAYKIT_CHECK(v.size() == 1 || v.size() == 2, ArgumentException("str", str));
			if (v.size() == 1)
				return stingray::FromString<value_type>(v[0]);
			else
			{
				fixed_point integral_part = stingray::FromString<value_type>(v[0]);
				fixed_point fractional_part = stingray::FromString<value_type>(v[1]);
				for (size_t i = 0; i < v[1].size(); ++i)
					fractional_part /= 10;
				return integral_part + fractional_part;
			}
		}
	};


}

#endif
