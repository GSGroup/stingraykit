#include <stingraykit/collection/Range.h>
#include <stingraykit/function/bind.h>
#include <stingraykit/function/functional.h>

#include <gtest/gtest.h>

using namespace stingray;

namespace
{

	template<typename R_, typename It_>
	void CheckSequence(R_ r, It_ seqBegin, It_ seqEnd, bool finite)
	{
		for (; r.Valid() && seqBegin != seqEnd; r.Next(), ++seqBegin)
			ASSERT_EQ(r.Get(), *seqBegin);
		ASSERT_TRUE(finite ? !r.Valid() : r.Valid());
		ASSERT_TRUE(seqBegin == seqEnd);
	}


	template<typename RIt_, typename It_>
	void CheckIterators(RIt_ rBegin, RIt_ rEnd, It_ seqBegin, It_ seqEnd)
	{
		for (; rBegin != rEnd && seqBegin != seqEnd; ++rBegin, ++seqBegin)
			ASSERT_EQ(*rBegin, *seqBegin);
		ASSERT_TRUE(rBegin == rEnd);
		ASSERT_TRUE(seqBegin == seqEnd);
	}


	template<typename R_, typename It_, typename Category_>
	void CheckRangeImpl(R_ r, It_ seqBegin, It_ seqEnd, const Category_& category)
	{
		CheckSequence(r, seqBegin, seqEnd, true);
		CheckIterators(r.begin(), r.end(), seqBegin, seqEnd);
	}


	template<typename R_, typename It_>
	void CheckRangeImpl(R_ r, It_ seqBegin, It_ seqEnd, const std::random_access_iterator_tag& category)
	{
		CheckSequence(r, seqBegin, seqEnd, true);
		CheckIterators(r.begin(), r.end(), seqBegin, seqEnd);

		std::ptrdiff_t size = seqEnd - seqBegin;
		ASSERT_EQ(std::ptrdiff_t(r.GetSize()), size);
		ASSERT_EQ(r.end() - r.begin(), size);
	}


	template<typename R_, typename It_>
	void CheckRange(R_ r, It_ seqBegin, It_ seqEnd)
	{ CheckRangeImpl(r, seqBegin, seqEnd, typename R_::Category()); }


	std::string DoZip(int a, const std::string& b, bool c)
	{ return ToString(a) + b + ToString(c); }


	struct Base
	{
		virtual ~Base()
		{ }
	};
	STINGRAYKIT_DECLARE_PTR(Base);


	struct Derived1 : public Base
	{
		int _value;

		Derived1(int value) : _value(value)
		{ }

		int GetValue() const
		{ return _value; }
	};
	STINGRAYKIT_DECLARE_PTR(Derived1);


	struct Derived2 : public Base
	{
		int _value;

		Derived2(int value) : _value(value)
		{ }

		int GetValue() const
		{ return _value; }
	};
	STINGRAYKIT_DECLARE_PTR(Derived2);


	struct InvalidRange : public Range::RangeBase<InvalidRange, bool, std::forward_iterator_tag>
	{
		typedef RangeBase<InvalidRange, int, std::forward_iterator_tag> base;
		typedef InvalidRange Self;

		bool Valid() const             			{ return true; }
		typename base::ValueType Get() const	{ return true; }

		bool Equals(const Self& other) const	{ return false; }
		Self& First()							{ return *this = Self(); }
		Self& Last()							{ return Next(); }
		Self& Next()                		    { STINGRAYKIT_THROW("Must not iterate"); }
	};


	template < typename Range_ >
	void StartIterate(Range_ range)
	{
		for (int i : range)
			break;
	}

}

TEST(RangeTest, Ranges)
{
	int initialSequence[] = {1, -42, 5, 42, 42, 37, 0, 53};
	std::vector<int> v(std::begin(initialSequence), std::end(initialSequence));
	std::set<int> s(std::begin(initialSequence), std::end(initialSequence));

	{
		int seq[] = {1, -42, 5, 42, 42, 37, 0, 53};
		CheckRange(ToRange(initialSequence), std::begin(seq), std::end(seq));
	}

	{
		int seq[] = {1, -42, 5, 42, 42, 37, 0, 53};
		CheckRange(ToRange(v), std::begin(seq), std::end(seq));
	}

	{
		int seq[] = {1, -42, 5, 42, 42, 37, 0, 53};
		ASSERT_EQ(std::ptrdiff_t(ToRange(seq) | Count()), std::distance(std::begin(seq), std::end(seq)));
	}

	{
		int seq[] = {1, -42, 5, 37, 0, 53};
		CheckRange(ToRange(v) | Filter(not_(Bind(comparers::Equals(), 42, _1))), std::begin(seq), std::end(seq));
	}

	{
		int seq[] = {53, 0, 37, 42, 42, 5, -42, 1};
		CheckRange(ToRange(v) | Reverse(), std::begin(seq), std::end(seq));
	}

	{
		int seq[] = {2, -41, 6, 43, 43, 38, 1, 54};
		CheckRange(ToRange(v) | Transform(Bind(std::plus<int>(), _1, 1)), std::begin(seq), std::end(seq));
	}

	{
		int seq[] = {54, 1, 38, 43, 43, -41, 2};
		CheckRange(ToRange(v) | Filter(not_(Bind(comparers::Equals(), 5, _1))) | Transform(Bind(std::plus<int>(), _1, 1)) | Reverse(), std::begin(seq), std::end(seq));
	}

	{
		int seq[] = {1, -42, 5, 42, 42, 37, 0, 53};
		CheckRange(ToRange(v) | Drop(0), std::begin(seq), std::end(seq));
	}

	{
		int seq[] = {42, 37, 0, 53};
		CheckRange(ToRange(v) | Drop(4), std::begin(seq), std::end(seq));
	}

	{
		int val;
		CheckRange(ToRange(v) | Drop(8), &val, &val);
	}

	{
		int val;
		CheckRange(ToRange(v) | Drop(16), &val, &val);
	}

	{
		int val;
		CheckRange(ToRange(v) | Take(0), &val, &val);
	}

	{
		int val;
		CheckRange(ToRange(&val, &val) | Take(4), &val, &val);
	}

	{
		int seq[] = {1, -42, 5, 42};
		CheckRange(ToRange(v) | Take(4), std::begin(seq), std::end(seq));
	}

	{
		int seq[] = {1, -42, 5, 42, 42, 37, 0, 53};
		CheckRange(ToRange(v) | Take(16), std::begin(seq), std::end(seq));
	}

	{
		int seq[] = {-42, 0, 1, 5, 37, 42, 53};
		CheckRange(ToRange(s), std::begin(seq), std::end(seq));
	}

	ASSERT_EQ(Range::Sum(ToRange(v)), 138);

	ASSERT_EQ(Range::ElementAt(ToRange(v), 0), 1);
	ASSERT_EQ(Range::ElementAt(ToRange(v), 5), 37);
	ASSERT_EQ(Range::ElementAt(ToRange(v), 7), 53);
	ASSERT_THROW(Range::ElementAt(ToRange(v), 10), IndexOutOfRangeException);

	ASSERT_EQ(Range::ElementAtOrDefault(ToRange(v), 0), 1);
	ASSERT_EQ(Range::ElementAtOrDefault(ToRange(v), 5), 37);
	ASSERT_EQ(Range::ElementAtOrDefault(ToRange(v), 7), 53);
	ASSERT_EQ(Range::ElementAtOrDefault(ToRange(v), 10), 0);

	ASSERT_EQ(Range::MinElement(ToRange(v)), -42);
	ASSERT_EQ(Range::MaxElement(ToRange(v)), 53);
	ASSERT_EQ(Fold(ToRange(v), 0, std::plus<int>()), 138); // TODO: add FoldTransformer
	ASSERT_EQ(Fold(ToRange(v), std::plus<int>()), 138);
	ASSERT_TRUE(ToRange(v) | Filter(Bind(comparers::Equals(), 5, _1)) | Any());

	ASSERT_EQ((ToRange(s) += 3).Get(), 5);
}


TEST(RangeTest, Cycle)
{
	int pattern[] = {1, -42, 5};

	int seq[] = {1, -42, 5, 1, -42, 5, 1, -42, 5, 1, -42, 5, 1, -42, 5, 1, -42, 5, 1, -42, 5};
	CheckSequence(Cycle(ToRange(pattern)), std::begin(seq), std::end(seq), false); // TODO: add CycleTransformer
}


TEST(RangeTest, Zip)
{
	{
		int r1[] = { 1, 2, 3 };
		int r2[] = { 4, 5, 6 };

		int seq[] = { 5, 7, 9 };
		CheckSequence(Range::Zip(std::plus<int>(), ToRange(r1), ToRange(r2)), std::begin(seq), std::end(seq), true);

		int rseq[] = { 9, 7, 5 };
		CheckSequence(Range::Zip(std::plus<int>(), ToRange(r1), ToRange(r2)) | Reverse(), std::begin(rseq), std::end(rseq), true);
	}

	{
		int r1[] = { 1, 2, 3 };
		int r2[] = { 4, 5, 6, 7 };

		int seq[] = { 5, 7, 9 };
		CheckSequence(Range::Zip(std::plus<int>(), ToRange(r1), ToRange(r2)), std::begin(seq), std::end(seq), true);

		int rseq[] = { 9, 7, 5 };
		CheckSequence(Range::Zip(std::plus<int>(), ToRange(r1), ToRange(r2)) | Reverse(), std::begin(rseq), std::end(rseq), true);
	}

	{
		int r1[] = { 1, 2 };
		int r2[] = { 4, 5, 6, 7, 8 };

		int seq[] = { 5, 7, 7, 9, 9 };
		CheckSequence(Range::Zip(std::plus<int>(), Cycle(ToRange(r1)), ToRange(r2)), std::begin(seq), std::end(seq), true);

		int rseq[] = { 9, 9, 7, 7, 5 };
		CheckSequence(Range::Zip(std::plus<int>(), Cycle(ToRange(r1)), ToRange(r2)) | Reverse(), std::begin(rseq), std::end(rseq), true);
	}

	{
		int a[] = { 1, 2, 3 };
		std::string b[] = { "a", "b", "c" };
		bool c[] = { true, false, true };

		std::string seq[] = { "1atrue", "2bfalse", "3ctrue" };
		CheckSequence(Range::Zip(&DoZip, ToRange(a), ToRange(b), ToRange(c)), std::begin(seq), std::end(seq), true);
	}
}


TEST(RangeTest, Split)
{
	int r[] = { 1, 2, 3, 4, 5, 6, 7, 8, 9 };

	ASSERT_EQ(Range::Split(std::begin(r), std::end(r), 1) | Count(), 9u);
	ASSERT_EQ(Range::Split(std::begin(r), std::end(r), 2) | Count(), 5u);
	ASSERT_EQ(Range::Split(std::begin(r), std::end(r), 3) | Count(), 3u);
	ASSERT_EQ(Range::Split(std::begin(r), std::end(r), 10) | Count(), 1u);
}


TEST(RangeTest, Polymorphic)
{
	std::vector<BasePtr> v;
	v.push_back(make_shared_ptr<Derived1>(1));
	v.push_back(make_shared_ptr<Derived1>(2));
	v.push_back(make_shared_ptr<Derived1>(3));
	v.push_back(make_shared_ptr<Derived1>(42));

	{
		int seq[] = {1, 2, 3, 42};
		CheckRange(ToRange(v) | Cast<Derived1Ptr>() | Transform(&Derived1::GetValue), std::begin(seq), std::end(seq));
	}

	ASSERT_TRUE((ToRange(v) | Cast<Derived2Ptr>()).Valid());
	ASSERT_ANY_THROW((ToRange(v) | Cast<Derived2Ptr>()).Get());

	v.push_back(make_shared_ptr<Derived2>(53));
	v.push_back(make_shared_ptr<Derived2>(1));
	v.push_back(make_shared_ptr<Derived1>(37));
	v.push_back(make_shared_ptr<Derived2>(7));

	{
		int seq[] = {1, 2, 3, 42, 37};
		CheckRange(ToRange(v) | OfType<Derived1Ptr>() | Transform(&Derived1::GetValue), std::begin(seq), std::end(seq));
	}

	{
		int seq[] = {53, 1, 7};
		CheckRange(ToRange(v) | OfType<Derived2Ptr>() | Transform(&Derived2::GetValue), std::begin(seq), std::end(seq));
	}
}


TEST(RangeTest, ForBasedLoop)
{
	ASSERT_ANY_THROW(StartIterate(InvalidRange()));
	ASSERT_NO_THROW(StartIterate(IterableRange(InvalidRange())));

	{
		int r[] = { 0, 1, 2, 3, 4 };
		int j = 0;
		for (int i : IterableRange(ToRange(r)))
			ASSERT_EQ(j++, i);

		ASSERT_EQ(j, 5);
	}
}
