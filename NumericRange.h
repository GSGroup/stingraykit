#ifndef __GS_DVRLIB_TOOLKIT_NUMERICRANGE_H__
#define	__GS_DVRLIB_TOOLKIT_NUMERICRANGE_H__

#include <dvrlib/toolkit/exception.h>

namespace dvrlib {

template<typename T>
class NumericRange
{
public:
	typedef T ValueT;
private:
	T _start, _end;
public:
	NumericRange(T start, T end) : _start(start), _end(end)
	{ TOOLKIT_CHECK(start <= end, ArgumentException("Start is greater than end")); }

	T GetStart() const		{ return _start; }
	void SetStart(T value)	{ _start = value; }
	T GetEnd() const		{ return _end; }
	void SetEnd(T value)	{ _end = value; }

	bool Contains(T t) const		{ return !IsEmpty() && _start <= t && t <= _end; }
	bool ContainsStrict(T t) const	{ return _start < t && t < _end; }

	bool IsEmpty() const			{ return _start == _end; }
	static NumericRange<T> CreateEmpty()
	{
		T t = T();
		return NumericRange(t, t);
	}

	bool IsIntersecting(const NumericRange<T>& other) const
	{
		if (IsEmpty() || other.IsEmpty())
			return false;
		return (_start < other._start && other._start < _end) || (_start < other._end && other._end < _end);
	}

	NumericRange<T> Intersect(const NumericRange<T>& other) const
	{
		if (IsEmpty())
			return other;
		if (other.IsEmpty())
			return *this;
		T start = std::max(_start, other._start);
		T end = std::min(_end, other._end);
		if (start >= end)
			return CreateEmpty();
		return NumericRange(start, end);
	}

	NumericRange<T> Union(const NumericRange<T>& other) const
	{
		if (IsEmpty())
			return other;
		if (other.IsEmpty())
			return *this;
		T start = std::min(_start, other._start);
		T end = std::max(_end, other._end);
		return NumericRange(start, end);
	}

	std::string ToString() const
	{
		return "[" + _start.ToString() + " - " + _end.ToString()  + "]";
	}
};


}

#endif
