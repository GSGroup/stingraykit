#ifndef STINGRAYKIT_COLLECTION_RANGEBASE_H
#define STINGRAYKIT_COLLECTION_RANGEBASE_H

#include <stingraykit/MetaProgramming.h>
#include <stingraykit/toolkit.h>

#include <iterator>

namespace stingray
{

	struct RangeMarker
	{
	protected:
		RangeMarker()
		{ }
	};


	template<typename T>
	struct IsRange
	{ static const bool Value = Inherits<T, RangeMarker>::Value; };


	namespace Range
	{
		template<typename T>
		struct ArrowProxy
		{
		private:
			T _val;

		public:
			ArrowProxy(T val) : _val(val)
			{ }

			typename ToPointerType<T>::ValueT operator -> ()
			{ return to_pointer(_val); }
		};


		template<typename Derived_, typename ValueType_, typename Category_>
		struct RangeBase :
			public std::iterator<Category_, ValueType_, std::ptrdiff_t, ArrowProxy<ValueType_>, ValueType_>,
			public RangeMarker
		{
		private:
			typedef Derived_   Derived;

		public:
			typedef Category_  Category;
			typedef ValueType_ ValueType;

		public:
			ValueType      operator *  ()                              { return GetDerived().Get(); }
			ArrowProxy<ValueType> operator -> ()                       { return GetDerived().Get(); }

			Derived&       operator ++ ()                              { GetDerived().Next(); return GetDerived(); }
			Derived        operator ++ (int)                           { Derived result(GetDerived()); GetDerived().Next(); return result; }

			bool           operator == (const Derived& other) const    { return GetDerived().Equals(other); }
			bool           operator != (const Derived& other) const    { return !GetDerived().Equals(other); }

			Derived&       operator -- ()                              { GetDerived().Prev(); return GetDerived(); }
			Derived        operator -- (int)                           { Derived result(GetDerived()); GetDerived().Prev(); return result; }

			ValueType      operator [] (std::ptrdiff_t index) const    { CompileTimeAssert<false> errorNoBracketsOperator; (void)errorNoBracketsOperator; }
			Derived&       operator += (std::ptrdiff_t distance)       { GetDerived().Move(distance); return GetDerived(); }
			Derived        operator +  (std::ptrdiff_t distance) const { Derived result(GetDerived()); return result += distance; }
			Derived&       operator -= (std::ptrdiff_t distance)       { GetDerived().Move(distance); return GetDerived(); }
			Derived        operator -  (std::ptrdiff_t distance) const { Derived result(GetDerived()); return result -= distance; }
			std::ptrdiff_t operator -  (const Derived& other) const    { return GetDerived().GetPosition() - other.GetPosition(); }

			Derived begin() const
			{ Derived result(GetDerived()); return result.First(); }

			Derived end() const
			{ Derived result(GetDerived()); result.Last(); return result.Next(); }

		private:
			Derived& GetDerived()                                      { return *static_cast<Derived*>(this); }
			const Derived& GetDerived() const                          { return *static_cast<const Derived*>(this); }
		};
	}
}

#endif
