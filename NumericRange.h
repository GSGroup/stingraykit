#ifndef __GS_DVRLIB_TOOLKIT_NUMERICRANGE_H__
#define	__GS_DVRLIB_TOOLKIT_NUMERICRANGE_H__

#include <stingray/toolkit/exception.h>

namespace stingray {

	template<typename DerivedT, typename T>
	class NumericRange
	{
	public:
		typedef T ValueT;

	private:
		T _start, _end;

	protected:
		NumericRange(T start, T end) : _start(start), _end(end)
		{ TOOLKIT_CHECK(start <= end, ArgumentException("Start is greater than end")); }

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
				return *this;
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
			return "[" + _start.ToString() + " - " + _end.ToString()  + "]";
		}

		bool operator== (const DerivedT& rhs)	{ return _start == rhs._start && _end == rhs._end; }
		bool operator!= (const DerivedT& rhs)	{ return !(*this == rhs); }
	};


}

#endif
