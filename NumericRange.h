#ifndef STINGRAY_TOOLKIT_NUMERICRANGE_H
#define STINGRAY_TOOLKIT_NUMERICRANGE_H

#include <stingray/toolkit/exception.h>
#include <stingray/toolkit/string/StringUtils.h>

namespace stingray {

	template<typename DerivedT, typename T>
	class NumericRange
	{
	public:
		typedef T ValueT;

	private:
		T _start, _end;

	protected:
		NumericRange() : _start(), _end()
		{ }

		NumericRange(T start, T end) : _start(start), _end(end)
		{ TOOLKIT_CHECK(start <= end, ArgumentException("Start is greater than end " + ToString())); }

	public:
		T GetStart() const		{ return _start; }
		void SetStart(T value)	{ _start = value; }
		T GetEnd() const		{ return _end; }
		void SetEnd(T value)	{ _end = value; }

		bool Contains(T t) const		{ return !IsEmpty() && _start <= t && t <= _end; }
		bool ContainsStrict(T t) const	{ return _start < t && t < _end; }

		bool IsEmpty() const			{ return _start == _end; }
		static DerivedT CreateEmpty()
		{
			T t = T();
			return DerivedT(t, t);
		}

		bool IsIntersecting(const DerivedT& other) const
		{
			if (IsEmpty() || other.IsEmpty())
				return false;
			return (_start < other._start && other._start < _end) || (_start < other._end && other._end < _end);
		}

		DerivedT Intersect(const DerivedT& other) const
		{
			if (IsEmpty())
				return other;
			if (other.IsEmpty())
				return *static_cast<const DerivedT*>(this);
			T start = std::max(_start, other._start);
			T end = std::min(_end, other._end);
			if (start >= end)
				return CreateEmpty();
			return DerivedT(start, end);
		}

		DerivedT Union(const DerivedT& other) const
		{
			if (IsEmpty())
				return other;
			if (other.IsEmpty())
				return *static_cast<const DerivedT*>(this);
			T start = std::min(_start, other._start);
			T end = std::max(_end, other._end);
			return DerivedT(start, end);
		}

		std::string ToString() const
		{
			return StringBuilder() % "["  % _start % " - " % _end  % "]";
		}

		bool operator== (const DerivedT& rhs)	{ return _start == rhs._start && _end == rhs._end; }
		bool operator!= (const DerivedT& rhs)	{ return !(*this == rhs); }
	};

	template< typename T >
	struct SimpleNumericRange : public NumericRange<SimpleNumericRange<T>, T>
	{
		typedef NumericRange<SimpleNumericRange<T>, T> base;

		SimpleNumericRange()
		{}

		SimpleNumericRange(const base &b) : base(b)
		{}

		SimpleNumericRange(T start, T end) : base(start, end)
		{}
	};


}

#endif
