#ifndef STINGRAYKIT_NUMERICRANGE_H
#define STINGRAYKIT_NUMERICRANGE_H

// Copyright (c) 2011 - 2019, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/exception.h>
#include <stingraykit/string/ToString.h>

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
		{ STINGRAYKIT_CHECK(start <= end, ArgumentException("Start is greater than end " + ToString())); }

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
			return _start < other._end && other._start < _end;
		}

		DerivedT Intersect(const DerivedT& other) const
		{
			if (IsEmpty() || other.IsEmpty())
				return CreateEmpty();
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

		T Clamp(T t) const { return std::max(_start, std::min(t, _end)); }

		std::string ToString() const
		{
			return StringBuilder() % "["  % _start % " - " % _end  % "]";
		}

		bool operator== (const DerivedT& rhs) const	{ return _start == rhs._start && _end == rhs._end; }
		bool operator!= (const DerivedT& rhs) const	{ return !(*this == rhs); }

		template<typename OStream>
		void Serialize(OStream& ar) const
		{
			ar.Serialize("start", _start);
			ar.Serialize("end", _end);
		}

		template<typename IStream>
		void Deserialize(IStream& ar)
		{
			ar.Deserialize("start", _start);
			ar.Deserialize("end", _end);
		}
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

		T Distance() const { return base::GetEnd() - base::GetStart(); }
	};


}

#endif
