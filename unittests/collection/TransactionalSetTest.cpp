#include <stingraykit/collection/ForEach.h>
#include <stingraykit/collection/TransactionalSet.h>
#include <stingraykit/signal/ValueFromSignalObtainer.h>

#include <gtest/gtest.h>

using namespace stingray;

namespace
{

	struct DiffCollector
	{
		std::set<int> Added;
		std::set<int> Removed;

		void Do(const shared_ptr<IEnumerable<DiffEntry<int> > >& diff)
		{
			FOR_EACH(DiffEntry<int> de IN diff)
				if (de.Op == CollectionOp::Added)
					Added.insert(de.Item);
				else if (de.Op == CollectionOp::Removed)
					Removed.insert(de.Item);
				else
					STINGRAYKIT_THROW(NotSupportedException());
		}

		void Clear() { Added.clear(); Removed.clear(); }
	};

}

TEST(TransactionalSetTest, Test1)
{
	using SetType = ITransactionalSet<int>;

	shared_ptr<SetType> s = make_shared_ptr<TransactionalSet<int> >();

	ASSERT_TRUE(s->IsEmpty());
	ASSERT_EQ(Enumerable::Count(s->GetEnumerator()), 0u);
	ASSERT_EQ(s->GetCount(), 0u);

	DiffCollector dc;
	Token t = s->OnChanged().connect(Bind(&DiffCollector::Do, &dc, _1));

	shared_ptr<ISetTransaction<int> > tr = s->StartTransaction();
	tr->Add(1);
	tr->Commit();
	tr.reset();

	ASSERT_TRUE(!s->IsEmpty());
	ASSERT_EQ(s->GetCount(), 1u);
	ASSERT_EQ(dc.Added.size(), 1u);
	ASSERT_EQ(*dc.Added.begin(), 1);
	ASSERT_EQ(dc.Removed.size(), 0u);
	dc.Clear();
	{
		int seq[] = {1};
		DiffCollector dc1;
		Token t1 = s->OnChanged().connect(Bind(&DiffCollector::Do, &dc1, _1));
		ASSERT_TRUE(dc1.Added == std::set<int>(std::begin(seq), std::end(seq)));
		ASSERT_EQ(dc1.Removed.size(), 0u);
		ASSERT_TRUE(Enumerable::SequenceEqual(s, EnumerableFromStlIterators(std::begin(seq), std::end(seq))));
	}

	tr = s->StartTransaction();
	tr->Add(42);
	tr->Commit();
	tr.reset();

	ASSERT_TRUE(!s->IsEmpty());
	ASSERT_EQ(s->GetCount(), 2u);
	ASSERT_EQ(dc.Added.size(), 1u);
	ASSERT_EQ(*dc.Added.begin(), 42);
	ASSERT_EQ(dc.Removed.size(), 0u);
	dc.Clear();
	{
		int seq[] = {1, 42};
		DiffCollector dc1;
		Token t1 = s->OnChanged().connect(Bind(&DiffCollector::Do, &dc1, _1));
		ASSERT_TRUE(dc1.Added == std::set<int>(std::begin(seq), std::end(seq)));
		ASSERT_EQ(dc1.Removed.size(), 0u);
		ASSERT_TRUE(Enumerable::SequenceEqual(s, EnumerableFromStlIterators(std::begin(seq), std::end(seq))));
	}

	tr = s->StartTransaction();

	ASSERT_TRUE(!tr->IsEmpty());
	ASSERT_EQ(tr->GetCount(), 2u);
	{
		int seq[] = {1, 42};
		ASSERT_TRUE(Enumerable::SequenceEqual(tr, EnumerableFromStlIterators(std::begin(seq), std::end(seq))));
	}

	tr->Add(37);

	ASSERT_TRUE(!tr->IsEmpty());
	ASSERT_EQ(tr->GetCount(), 3u);
	{
		int seq[] = {1, 37, 42};
		ASSERT_TRUE(Enumerable::SequenceEqual(tr, EnumerableFromStlIterators(std::begin(seq), std::end(seq))));
	}

	ASSERT_EQ(dc.Added.size(), 0u);
	ASSERT_EQ(dc.Removed.size(), 0u);
	{
		int seq[] = {1, 42};
		DiffCollector dc1;
		Token t1 = s->OnChanged().connect(Bind(&DiffCollector::Do, &dc1, _1));
		ASSERT_TRUE(dc1.Added == std::set<int>(std::begin(seq), std::end(seq)));
		ASSERT_EQ(dc1.Removed.size(), 0u);
		ASSERT_TRUE(Enumerable::SequenceEqual(s, EnumerableFromStlIterators(std::begin(seq), std::end(seq))));
	}

	tr->Add(51);

	ASSERT_TRUE(!tr->IsEmpty());
	ASSERT_EQ(tr->GetCount(), 4u);
	{
		int seq[] = {1, 37, 42, 51};
		ASSERT_TRUE(Enumerable::SequenceEqual(tr, EnumerableFromStlIterators(std::begin(seq), std::end(seq))));
	}

	ASSERT_EQ(dc.Added.size(), 0u);
	ASSERT_EQ(dc.Removed.size(), 0u);
	{
		int seq[] = {1, 42};
		DiffCollector dc1;
		Token t1 = s->OnChanged().connect(Bind(&DiffCollector::Do, &dc1, _1));
		ASSERT_TRUE(dc1.Added == std::set<int>(std::begin(seq), std::end(seq)));
		ASSERT_EQ(dc1.Removed.size(), 0u);
		ASSERT_TRUE(Enumerable::SequenceEqual(s, EnumerableFromStlIterators(std::begin(seq), std::end(seq))));
	}

	tr->Commit();
	tr.reset();

	ASSERT_TRUE(!s->IsEmpty());
	ASSERT_EQ(s->GetCount(), 4u);
	ASSERT_EQ(dc.Added.size(), 2u);
	ASSERT_EQ(*dc.Added.begin(), 37);
	ASSERT_EQ(*std::next(dc.Added.begin()), 51);
	ASSERT_EQ(dc.Removed.size(), 0u);
	dc.Clear();
	{
		int seq[] = {1, 37, 42, 51};
		DiffCollector dc1;
		Token t1 = s->OnChanged().connect(Bind(&DiffCollector::Do, &dc1, _1));
		ASSERT_TRUE(dc1.Added == std::set<int>(std::begin(seq), std::end(seq)));
		ASSERT_EQ(dc1.Removed.size(), 0u);
		ASSERT_TRUE(Enumerable::SequenceEqual(s, EnumerableFromStlIterators(std::begin(seq), std::end(seq))));
	}

	tr = s->StartTransaction();
	tr->Remove(37);

	ASSERT_TRUE(!tr->IsEmpty());
	ASSERT_EQ(tr->GetCount(), 3u);
	{
		int seq[] = {1, 42, 51};
		ASSERT_TRUE(Enumerable::SequenceEqual(tr, EnumerableFromStlIterators(std::begin(seq), std::end(seq))));
	}

	ASSERT_EQ(dc.Added.size(), 0u);
	ASSERT_EQ(dc.Removed.size(), 0u);
	{
		int seq[] = {1, 37, 42, 51};
		DiffCollector dc1;
		Token t1 = s->OnChanged().connect(Bind(&DiffCollector::Do, &dc1, _1));
		ASSERT_TRUE(dc1.Added == std::set<int>(std::begin(seq), std::end(seq)));
		ASSERT_EQ(dc1.Removed.size(), 0u);
		ASSERT_TRUE(Enumerable::SequenceEqual(s, EnumerableFromStlIterators(std::begin(seq), std::end(seq))));
	}

	tr->Remove(42);

	ASSERT_TRUE(!tr->IsEmpty());
	ASSERT_EQ(tr->GetCount(), 2u);
	{
		int seq[] = {1, 51};
		ASSERT_TRUE(Enumerable::SequenceEqual(tr, EnumerableFromStlIterators(std::begin(seq), std::end(seq))));
	}

	ASSERT_EQ(dc.Added.size(), 0u);
	ASSERT_EQ(dc.Removed.size(), 0u);
	{
		int seq[] = {1, 37, 42, 51};
		DiffCollector dc1;
		Token t1 = s->OnChanged().connect(Bind(&DiffCollector::Do, &dc1, _1));
		ASSERT_TRUE(dc1.Added == std::set<int>(std::begin(seq), std::end(seq)));
		ASSERT_TRUE(Enumerable::SequenceEqual(s, EnumerableFromStlIterators(std::begin(seq), std::end(seq))));
	}

	tr->Add(13);

	ASSERT_TRUE(!tr->IsEmpty());
	ASSERT_EQ(tr->GetCount(), 3u);
	{
		int seq[] = {1, 13, 51};
		ASSERT_TRUE(Enumerable::SequenceEqual(tr, EnumerableFromStlIterators(std::begin(seq), std::end(seq))));
	}

	ASSERT_EQ(dc.Added.size(), 0u);
	ASSERT_EQ(dc.Removed.size(), 0u);
	{
		int seq[] = {1, 37, 42, 51};
		DiffCollector dc1;
		Token t1 = s->OnChanged().connect(Bind(&DiffCollector::Do, &dc1, _1));
		ASSERT_TRUE(dc1.Added == std::set<int>(std::begin(seq), std::end(seq)));
		ASSERT_TRUE(Enumerable::SequenceEqual(s, EnumerableFromStlIterators(std::begin(seq), std::end(seq))));
	}

	tr->Commit();
	tr.reset();

	ASSERT_TRUE(!s->IsEmpty());
	ASSERT_EQ(s->GetCount(), 3u);
	ASSERT_EQ(dc.Added.size(), 1u);
	ASSERT_EQ(*dc.Added.begin(), 13);
	ASSERT_EQ(dc.Removed.size(), 2u);
	ASSERT_EQ(*dc.Removed.begin(), 37);
	ASSERT_EQ(*std::next(dc.Removed.begin()), 42);
	dc.Clear();
	{
		int seq[] = {1, 13, 51};
		DiffCollector dc1;
		Token t1 = s->OnChanged().connect(Bind(&DiffCollector::Do, &dc1, _1));
		ASSERT_TRUE(dc1.Added == std::set<int>(std::begin(seq), std::end(seq)));
		ASSERT_EQ(dc1.Removed.size(), 0u);
		ASSERT_TRUE(Enumerable::SequenceEqual(s, EnumerableFromStlIterators(std::begin(seq), std::end(seq))));
	}
}

TEST(TransactionalSetTest, Test2)
{
	using SetType = ITransactionalSet<int>;
	using EnumerableTypePtr = shared_ptr<IEnumerable<SetType::ValueType>>;

	// 1

	const shared_ptr<SetType> s = make_shared_ptr<TransactionalSet<int>>();

	ASSERT_TRUE(s->IsEmpty());
	ASSERT_EQ(s->GetCount(), 0u);
	ASSERT_EQ(Enumerable::Count(s), 0u);

	ASSERT_TRUE(Enumerable::SequenceEqual(s, (EnumerableTypePtr)MakeEmptyEnumerable()));
	ASSERT_TRUE(Enumerable::SequenceEqual(s->Reverse(), (EnumerableTypePtr)MakeEmptyEnumerable()));

	ASSERT_FALSE(GetValueFromSignal(s->OnChanged()));

	// 2

	SetType::TransactionTypePtr tr = s->StartTransaction();
	ASSERT_EQ(tr->GetCount(), 0u);
	ASSERT_TRUE(Enumerable::SequenceEqual(tr, (EnumerableTypePtr)MakeEmptyEnumerable()));

	const int seq1[] = {1, 2, 3, 4, 5};
	const auto seq1En = EnumerableFromStlIterators(std::begin(seq1), std::end(seq1));
	const auto seq1Ren = EnumerableFromStlIterators(std::rbegin(seq1), std::rend(seq1));

	ASSERT_FALSE(tr->Contains(1));
	tr->Add(1);
	ASSERT_TRUE(tr->Contains(1));
	ASSERT_EQ(tr->GetCount(), 1u);
	ASSERT_TRUE(Enumerable::SequenceEqual(tr, Enumerable::Take(seq1En, 1)));

	ASSERT_FALSE(tr->Contains(2));
	tr->Add(2);
	ASSERT_TRUE(tr->Contains(2));
	ASSERT_EQ(tr->GetCount(), 2u);
	ASSERT_TRUE(Enumerable::SequenceEqual(tr, Enumerable::Take(seq1En, 2)));

	ASSERT_FALSE(tr->Contains(3));
	tr->Add(3);
	ASSERT_TRUE(tr->Contains(3));
	ASSERT_EQ(tr->GetCount(), 3u);
	ASSERT_TRUE(Enumerable::SequenceEqual(tr, Enumerable::Take(seq1En, 3)));

	ASSERT_FALSE(tr->Contains(4));
	tr->Add(4);
	ASSERT_TRUE(tr->Contains(4));
	ASSERT_EQ(tr->GetCount(), 4u);
	ASSERT_TRUE(Enumerable::SequenceEqual(tr, Enumerable::Take(seq1En, 4)));

	ASSERT_FALSE(tr->Contains(5));
	tr->Add(5);
	ASSERT_TRUE(tr->Contains(5));
	ASSERT_EQ(tr->GetCount(), 5u);
	ASSERT_TRUE(Enumerable::SequenceEqual(tr, seq1En));

	SetType::DiffTypePtr diff1 = tr->Diff();

	ASSERT_FALSE(tr->Contains(6));
	tr->Remove(6);
	ASSERT_FALSE(tr->Contains(6));
	ASSERT_EQ(tr->GetCount(), 5u);
	ASSERT_TRUE(Enumerable::SequenceEqual(tr, seq1En));

	const int seq2[] = {1, 3, 4, 5};
	const auto seq2En = EnumerableFromStlIterators(std::begin(seq2), std::end(seq2));

	ASSERT_TRUE(tr->Contains(2));
	tr->Remove(2);
	ASSERT_FALSE(tr->Contains(2));
	ASSERT_EQ(tr->GetCount(), 4u);
	ASSERT_TRUE(Enumerable::SequenceEqual(tr, seq2En));

	const int seq3[] = {1, 3, 5};
	const auto seq3En = EnumerableFromStlIterators(std::begin(seq3), std::end(seq3));
	const auto seq3Ren = EnumerableFromStlIterators(std::rbegin(seq3), std::rend(seq3));

	ASSERT_TRUE(tr->Contains(4));
	tr->Remove(4);
	ASSERT_FALSE(tr->Contains(4));
	ASSERT_EQ(tr->GetCount(), 3u);
	ASSERT_TRUE(Enumerable::SequenceEqual(tr, seq3En));

	SetType::DiffTypePtr diff2 = tr->Diff();
	auto trEn1 = tr->GetEnumerator();

	tr->Commit();

	ASSERT_TRUE(Enumerable::SequenceEqual(diff1, WrapEnumerable(seq1En, Bind(&MakeDiffEntry<int>, CollectionOp::Added, _1)), comparers::Equals()));
	ASSERT_TRUE(Enumerable::SequenceEqual(diff2, WrapEnumerable(seq3En, Bind(&MakeDiffEntry<int>, CollectionOp::Added, _1)), comparers::Equals()));
	diff1.reset();
	diff2.reset();

	tr.reset();
	ASSERT_TRUE(Enumerable::SequenceEqual(trEn1, seq3En));
	trEn1.reset();

	// 3

	ASSERT_FALSE(s->IsEmpty());
	ASSERT_EQ(s->GetCount(), 3u);
	ASSERT_EQ(Enumerable::Count(s), 3u);

	ASSERT_TRUE(Enumerable::SequenceEqual(s, seq3En));
	ASSERT_TRUE(Enumerable::SequenceEqual(s->Reverse(), seq3Ren));

	ASSERT_TRUE(Enumerable::SequenceEqual(GetValueFromSignal(s->OnChanged()), WrapEnumerable(seq3En, Bind(&MakeDiffEntry<int>, CollectionOp::Added, _1)), comparers::Equals()));

	// 4

	tr = s->StartTransaction();
	ASSERT_EQ(tr->GetCount(), 3u);
	ASSERT_TRUE(Enumerable::SequenceEqual(tr, seq3En));

	const int seq4[] = {1, 2, 3, 5};
	const auto seq4En = EnumerableFromStlIterators(std::begin(seq4), std::end(seq4));

	ASSERT_FALSE(tr->Contains(2));
	tr->Add(2);
	ASSERT_TRUE(tr->Contains(2));
	ASSERT_EQ(tr->GetCount(), 4u);
	ASSERT_TRUE(Enumerable::SequenceEqual(tr, seq4En));

	ASSERT_FALSE(tr->Contains(4));
	tr->Add(4);
	ASSERT_TRUE(tr->Contains(4));
	ASSERT_EQ(tr->GetCount(), 5u);
	ASSERT_TRUE(Enumerable::SequenceEqual(tr, seq1En));

	diff1 = tr->Diff();

	const int seq5[] = {2, 3, 4, 5};
	const auto seq5En = EnumerableFromStlIterators(std::begin(seq5), std::end(seq5));

	ASSERT_TRUE(tr->Contains(1));
	tr->Remove(1);
	ASSERT_FALSE(tr->Contains(1));
	ASSERT_EQ(tr->GetCount(), 4u);
	ASSERT_TRUE(Enumerable::SequenceEqual(tr, seq5En));

	const int seq6[] = {2, 4, 5};
	const auto seq6En = EnumerableFromStlIterators(std::begin(seq6), std::end(seq6));

	ASSERT_TRUE(tr->Contains(3));
	tr->Remove(3);
	ASSERT_FALSE(tr->Contains(3));
	ASSERT_EQ(tr->GetCount(), 3u);
	ASSERT_TRUE(Enumerable::SequenceEqual(tr, seq6En));

	ASSERT_FALSE(tr->Contains(3));
	tr->Remove(3);
	ASSERT_FALSE(tr->Contains(3));
	ASSERT_EQ(tr->GetCount(), 3u);
	ASSERT_TRUE(Enumerable::SequenceEqual(tr, seq6En));

	const int seq7[] = {2, 4};
	const auto seq7En = EnumerableFromStlIterators(std::begin(seq7), std::end(seq7));
	const auto seq7Ren = EnumerableFromStlIterators(std::rbegin(seq7), std::rend(seq7));

	ASSERT_TRUE(tr->Contains(5));
	tr->Remove(5);
	ASSERT_FALSE(tr->Contains(5));
	ASSERT_EQ(tr->GetCount(), 2u);
	ASSERT_TRUE(Enumerable::SequenceEqual(tr, seq7En));

	ASSERT_FALSE(tr->Contains(5));
	tr->Remove(5);
	ASSERT_FALSE(tr->Contains(5));
	ASSERT_EQ(tr->GetCount(), 2u);
	ASSERT_TRUE(Enumerable::SequenceEqual(tr, seq7En));

	diff2 = tr->Diff();

	trEn1 = tr->GetEnumerator();
	tr->Commit();

	const DiffEntry<int> seq8[] = {{CollectionOp::Removed, 1}, {CollectionOp::Added, 2}, {CollectionOp::Removed, 3}, {CollectionOp::Added, 4}, {CollectionOp::Removed, 5}};
	const auto seq8En = EnumerableFromStlIterators(std::begin(seq8), std::end(seq8));

	ASSERT_TRUE(Enumerable::SequenceEqual(diff1, WrapEnumerable(seq7En, Bind(&MakeDiffEntry<int>, CollectionOp::Added, _1)), comparers::Equals()));
	ASSERT_TRUE(Enumerable::SequenceEqual(diff2, seq8En, comparers::Equals()));
	diff1.reset();
	diff2.reset();

	tr.reset();
	ASSERT_TRUE(Enumerable::SequenceEqual(trEn1, seq7En));
	trEn1.reset();

	// 5

	ASSERT_FALSE(s->IsEmpty());
	ASSERT_EQ(s->GetCount(), 2u);
	ASSERT_EQ(Enumerable::Count(s), 2u);

	ASSERT_TRUE(Enumerable::SequenceEqual(s, seq7En));
	ASSERT_TRUE(Enumerable::SequenceEqual(s->Reverse(), seq7Ren));

	ASSERT_TRUE(Enumerable::SequenceEqual(GetValueFromSignal(s->OnChanged()), WrapEnumerable(seq7En, Bind(&MakeDiffEntry<int>, CollectionOp::Added, _1)), comparers::Equals()));

	// 6

	tr = s->StartTransaction();
	ASSERT_TRUE(Enumerable::SequenceEqual(tr, seq7En));

	const int seq9[] = {1, 2, 4};
	const auto seq9En = EnumerableFromStlIterators(std::begin(seq9), std::end(seq9));

	ASSERT_FALSE(tr->Contains(1));
	tr->Add(1);
	ASSERT_TRUE(tr->Contains(1));
	ASSERT_TRUE(Enumerable::SequenceEqual(tr, seq9En));

	ASSERT_TRUE(tr->Contains(2));
	tr->Add(2);
	ASSERT_TRUE(tr->Contains(2));
	ASSERT_TRUE(Enumerable::SequenceEqual(tr, seq9En));

	const int seq10[] = {1, 2, 3, 4};
	const auto seq10En = EnumerableFromStlIterators(std::begin(seq10), std::end(seq10));

	ASSERT_FALSE(tr->Contains(3));
	tr->Add(3);
	ASSERT_TRUE(tr->Contains(3));
	ASSERT_TRUE(Enumerable::SequenceEqual(tr, seq10En));

	ASSERT_TRUE(tr->Contains(4));
	tr->Add(4);
	ASSERT_TRUE(tr->Contains(4));
	ASSERT_TRUE(Enumerable::SequenceEqual(tr, seq10En));

	ASSERT_FALSE(tr->Contains(5));
	tr->Add(5);
	ASSERT_TRUE(tr->Contains(5));
	ASSERT_TRUE(Enumerable::SequenceEqual(tr, seq1En));

	diff1 = tr->Diff();

	trEn1 = tr->GetEnumerator();
	tr->Commit();

	ASSERT_TRUE(Enumerable::SequenceEqual(diff1, WrapEnumerable(seq3En, Bind(&MakeDiffEntry<int>, CollectionOp::Added, _1)), comparers::Equals()));
	diff1.reset();

	tr.reset();
	ASSERT_TRUE(Enumerable::SequenceEqual(trEn1, seq1En));
	trEn1.reset();

	// 7

	ASSERT_FALSE(s->IsEmpty());
	ASSERT_EQ(s->GetCount(), 5u);
	ASSERT_EQ(Enumerable::Count(s), 5u);

	ASSERT_TRUE(Enumerable::SequenceEqual(s, seq1En));
	ASSERT_TRUE(Enumerable::SequenceEqual(s->Reverse(), seq1Ren));

	ASSERT_TRUE(Enumerable::SequenceEqual(GetValueFromSignal(s->OnChanged()), WrapEnumerable(seq1En, Bind(&MakeDiffEntry<int>, CollectionOp::Added, _1)), comparers::Equals()));

	// 8

	ASSERT_TRUE(Enumerable::SequenceEqual(s->Find(0), (EnumerableTypePtr)MakeEmptyEnumerable()));
	ASSERT_TRUE(Enumerable::SequenceEqual(s->Find(1), seq1En));
	ASSERT_TRUE(Enumerable::SequenceEqual(s->Find(2), Enumerable::Skip(seq1En, 1)));
	ASSERT_TRUE(Enumerable::SequenceEqual(s->Find(3), Enumerable::Skip(seq1En, 2)));
	ASSERT_TRUE(Enumerable::SequenceEqual(s->Find(4), Enumerable::Skip(seq1En, 3)));
	ASSERT_TRUE(Enumerable::SequenceEqual(s->Find(5), Enumerable::Skip(seq1En, 4)));
	ASSERT_TRUE(Enumerable::SequenceEqual(s->Find(6), (EnumerableTypePtr)MakeEmptyEnumerable()));

	ASSERT_TRUE(Enumerable::SequenceEqual(s->ReverseFind(0), (EnumerableTypePtr)MakeEmptyEnumerable()));
	ASSERT_TRUE(Enumerable::SequenceEqual(s->ReverseFind(1), Enumerable::Skip(seq1Ren, 4)));
	ASSERT_TRUE(Enumerable::SequenceEqual(s->ReverseFind(2), Enumerable::Skip(seq1Ren, 3)));
	ASSERT_TRUE(Enumerable::SequenceEqual(s->ReverseFind(3), Enumerable::Skip(seq1Ren, 2)));
	ASSERT_TRUE(Enumerable::SequenceEqual(s->ReverseFind(4), Enumerable::Skip(seq1Ren, 1)));
	ASSERT_TRUE(Enumerable::SequenceEqual(s->ReverseFind(5), seq1Ren));
	ASSERT_TRUE(Enumerable::SequenceEqual(s->ReverseFind(6), (EnumerableTypePtr)MakeEmptyEnumerable()));

	// 9

	tr = s->StartTransaction();
	ASSERT_TRUE(Enumerable::SequenceEqual(tr, seq1En));
	ASSERT_TRUE(Enumerable::SequenceEqual(tr->Reverse(), seq1Ren));

	const int seq11[] = {0, 1, 2, 3, 4, 5};
	const auto seq11En = EnumerableFromStlIterators(std::begin(seq11), std::end(seq11));
	const auto seq11Ren = EnumerableFromStlIterators(std::rbegin(seq11), std::rend(seq11));

	ASSERT_FALSE(tr->Contains(0));
	tr->Add(0);
	ASSERT_TRUE(tr->Contains(0));
	ASSERT_TRUE(Enumerable::SequenceEqual(tr, seq11En));
	ASSERT_TRUE(Enumerable::SequenceEqual(tr->Reverse(), seq11Ren));

	const int seq12[] = {0, 1, 3, 4, 5};
	const auto seq12En = EnumerableFromStlIterators(std::begin(seq12), std::end(seq12));
	const auto seq12Ren = EnumerableFromStlIterators(std::rbegin(seq12), std::rend(seq12));

	ASSERT_TRUE(tr->Contains(2));
	tr->Remove(2);
	ASSERT_FALSE(tr->Contains(2));
	ASSERT_TRUE(Enumerable::SequenceEqual(tr, seq12En));
	ASSERT_TRUE(Enumerable::SequenceEqual(tr->Reverse(), seq12Ren));

	const int seq13[] = {0, 1, 3, 5};
	const auto seq13En = EnumerableFromStlIterators(std::begin(seq13), std::end(seq13));
	const auto seq13Ren = EnumerableFromStlIterators(std::rbegin(seq13), std::rend(seq13));

	ASSERT_TRUE(tr->Contains(4));
	tr->Remove(4);
	ASSERT_FALSE(tr->Contains(2));
	ASSERT_TRUE(Enumerable::SequenceEqual(tr, seq13En));
	ASSERT_TRUE(Enumerable::SequenceEqual(tr->Reverse(), seq13Ren));

	const int seq14[] = {0, 1, 3, 5, 6};
	const auto seq14En = EnumerableFromStlIterators(std::begin(seq14), std::end(seq14));
	const auto seq14Ren = EnumerableFromStlIterators(std::rbegin(seq14), std::rend(seq14));

	ASSERT_FALSE(tr->Contains(6));
	tr->Add(6);
	ASSERT_TRUE(tr->Contains(6));
	ASSERT_TRUE(Enumerable::SequenceEqual(tr, seq14En));
	ASSERT_TRUE(Enumerable::SequenceEqual(tr->Reverse(), seq14Ren));

	diff1 = tr->Diff();

	// 10

	ASSERT_TRUE(Enumerable::SequenceEqual(tr->Find(-1), (EnumerableTypePtr)MakeEmptyEnumerable()));
	ASSERT_TRUE(Enumerable::SequenceEqual(tr->Find(0), seq14En));
	ASSERT_TRUE(Enumerable::SequenceEqual(tr->Find(1), Enumerable::Skip(seq14En, 1)));
	ASSERT_TRUE(Enumerable::SequenceEqual(tr->Find(2), (EnumerableTypePtr)MakeEmptyEnumerable()));
	ASSERT_TRUE(Enumerable::SequenceEqual(tr->Find(3), Enumerable::Skip(seq14En, 2)));
	ASSERT_TRUE(Enumerable::SequenceEqual(tr->Find(4), (EnumerableTypePtr)MakeEmptyEnumerable()));
	ASSERT_TRUE(Enumerable::SequenceEqual(tr->Find(5), Enumerable::Skip(seq14En, 3)));
	ASSERT_TRUE(Enumerable::SequenceEqual(tr->Find(6), Enumerable::Skip(seq14En, 4)));
	ASSERT_TRUE(Enumerable::SequenceEqual(tr->Find(7), (EnumerableTypePtr)MakeEmptyEnumerable()));

	ASSERT_TRUE(Enumerable::SequenceEqual(tr->ReverseFind(-1), (EnumerableTypePtr)MakeEmptyEnumerable()));
	ASSERT_TRUE(Enumerable::SequenceEqual(tr->ReverseFind(0), Enumerable::Skip(seq14Ren, 4)));
	ASSERT_TRUE(Enumerable::SequenceEqual(tr->ReverseFind(1), Enumerable::Skip(seq14Ren, 3)));
	ASSERT_TRUE(Enumerable::SequenceEqual(tr->ReverseFind(2), (EnumerableTypePtr)MakeEmptyEnumerable()));
	ASSERT_TRUE(Enumerable::SequenceEqual(tr->ReverseFind(3), Enumerable::Skip(seq14Ren, 2)));
	ASSERT_TRUE(Enumerable::SequenceEqual(tr->ReverseFind(4), (EnumerableTypePtr)MakeEmptyEnumerable()));
	ASSERT_TRUE(Enumerable::SequenceEqual(tr->ReverseFind(5), Enumerable::Skip(seq14Ren, 1)));
	ASSERT_TRUE(Enumerable::SequenceEqual(tr->ReverseFind(6), seq14Ren));
	ASSERT_TRUE(Enumerable::SequenceEqual(tr->ReverseFind(7), (EnumerableTypePtr)MakeEmptyEnumerable()));

	// 11

	tr->Commit();
	tr.reset();

	const DiffEntry<int> seq15[] = {{CollectionOp::Added, 0}, {CollectionOp::Removed, 2}, {CollectionOp::Removed, 4}, {CollectionOp::Added, 6}};
	const auto seq15En = EnumerableFromStlIterators(std::begin(seq15), std::end(seq15));

	ASSERT_TRUE(Enumerable::SequenceEqual(diff1, seq15En, comparers::Equals()));
	diff1.reset();

	// 12

	ASSERT_FALSE(s->IsEmpty());
	ASSERT_EQ(s->GetCount(), 5u);
	ASSERT_EQ(Enumerable::Count(s), 5u);

	ASSERT_TRUE(Enumerable::SequenceEqual(s, seq14En));
	ASSERT_TRUE(Enumerable::SequenceEqual(s->Reverse(), seq14Ren));

	ASSERT_TRUE(Enumerable::SequenceEqual(GetValueFromSignal(s->OnChanged()), WrapEnumerable(seq14En, Bind(&MakeDiffEntry<int>, CollectionOp::Added, _1)), comparers::Equals()));

	// 13

	tr = s->StartTransaction();
	ASSERT_TRUE(Enumerable::SequenceEqual(tr, seq14En));
	ASSERT_TRUE(Enumerable::SequenceEqual(tr->Reverse(), seq14Ren));

	ASSERT_TRUE(tr->Contains(1));
	tr->Remove(1);
	ASSERT_FALSE(tr->Contains(1));

	ASSERT_TRUE(tr->Contains(3));
	tr->Remove(3);
	ASSERT_FALSE(tr->Contains(3));

	ASSERT_TRUE(tr->Contains(5));
	tr->Remove(5);
	ASSERT_FALSE(tr->Contains(5));

	const DiffEntry<int> seq16[] = {{CollectionOp::Removed, 1}, {CollectionOp::Removed, 3}, {CollectionOp::Removed, 5}};
	const auto seq16En = EnumerableFromStlIterators(std::begin(seq16), std::end(seq16));

	diff1 = tr->Diff();
	tr->Revert();
	diff2 = tr->Diff();
	tr.reset();

	ASSERT_TRUE(Enumerable::SequenceEqual(diff1, seq16En, comparers::Equals()));
	diff1.reset();

	ASSERT_TRUE(Enumerable::SequenceEqual(diff2, (SetType::DiffTypePtr)MakeEmptyEnumerable(), comparers::Equals()));
	diff2.reset();

	// 14

	ASSERT_FALSE(s->IsEmpty());
	ASSERT_EQ(s->GetCount(), 5u);
	ASSERT_EQ(Enumerable::Count(s), 5u);

	ASSERT_TRUE(Enumerable::SequenceEqual(s, seq14En, comparers::Equals()));
	ASSERT_TRUE(Enumerable::SequenceEqual(s->Reverse(), seq14Ren, comparers::Equals()));

	ASSERT_TRUE(Enumerable::SequenceEqual(GetValueFromSignal(s->OnChanged()), WrapEnumerable(seq14En, Bind(&MakeDiffEntry<int>, CollectionOp::Added, _1)), comparers::Equals()));

	// 15
	// 16
	// 17
	// 18

	// 19

	tr = s->StartTransaction();
	ASSERT_TRUE(Enumerable::SequenceEqual(tr, seq14En, comparers::Equals()));
	ASSERT_TRUE(Enumerable::SequenceEqual(tr->Reverse(), seq14Ren, comparers::Equals()));

	const DiffEntry<int> seq19[] = {{CollectionOp::Added, 2}, {CollectionOp::Added, 4}};
	const auto seq19En = EnumerableFromStlIterators(std::begin(seq19), std::end(seq19));
	const auto seq19Ren = EnumerableFromStlIterators(std::rbegin(seq19), std::rend(seq19));

	ASSERT_FALSE(tr->Contains(2));
	tr->Add(2);
	ASSERT_TRUE(tr->Contains(2));
	ASSERT_TRUE(Enumerable::SequenceEqual(tr->Diff(), Enumerable::Take(seq19En, 1), comparers::Equals()));

	ASSERT_FALSE(tr->Contains(4));
	tr->Add(4);
	ASSERT_TRUE(tr->Contains(4));
	ASSERT_TRUE(Enumerable::SequenceEqual(tr->Diff(), seq19En, comparers::Equals()));

	ASSERT_TRUE(tr->Contains(2));
	ASSERT_TRUE(tr->TryRemove(2));
	ASSERT_FALSE(tr->Contains(2));
	ASSERT_FALSE(tr->TryRemove(2));
	ASSERT_FALSE(tr->Contains(2));
	ASSERT_TRUE(Enumerable::SequenceEqual(tr->Diff(), Enumerable::Skip(seq19En, 1), comparers::Equals()));

	ASSERT_TRUE(tr->Contains(4));
	ASSERT_TRUE(tr->TryRemove(4));
	ASSERT_FALSE(tr->Contains(4));
	ASSERT_FALSE(tr->TryRemove(4));
	ASSERT_FALSE(tr->Contains(4));
	ASSERT_TRUE(Enumerable::SequenceEqual(tr->Diff(), (SetType::DiffTypePtr)MakeEmptyEnumerable(), comparers::Equals()));

	tr->Commit();
	tr.reset();

	// 20

	ASSERT_FALSE(s->IsEmpty());
	ASSERT_EQ(s->GetCount(), 5u);
	ASSERT_EQ(Enumerable::Count(s), 5u);

	ASSERT_TRUE(Enumerable::SequenceEqual(s, seq14En, comparers::Equals()));
	ASSERT_TRUE(Enumerable::SequenceEqual(s->Reverse(), seq14Ren, comparers::Equals()));

	ASSERT_TRUE(Enumerable::SequenceEqual(GetValueFromSignal(s->OnChanged()), WrapEnumerable(seq14En, Bind(&MakeDiffEntry<int>, CollectionOp::Added, _1)), comparers::Equals()));

	// 21

	tr = s->StartTransaction();
	ASSERT_TRUE(Enumerable::SequenceEqual(tr, seq14En, comparers::Equals()));
	ASSERT_TRUE(Enumerable::SequenceEqual(tr->Reverse(), seq14Ren, comparers::Equals()));

	const DiffEntry<int> seq20[] = {{CollectionOp::Removed, 0}, {CollectionOp::Removed, 6}};
	const auto seq20En = EnumerableFromStlIterators(std::begin(seq20), std::end(seq20));
	const auto seq20Ren = EnumerableFromStlIterators(std::rbegin(seq20), std::rend(seq20));

	ASSERT_TRUE(tr->Contains(0));
	ASSERT_TRUE(tr->TryRemove(0));
	ASSERT_FALSE(tr->Contains(0));
	ASSERT_FALSE(tr->TryRemove(0));
	ASSERT_FALSE(tr->Contains(0));
	ASSERT_TRUE(Enumerable::SequenceEqual(tr->Diff(), Enumerable::Take(seq20En, 1), comparers::Equals()));

	ASSERT_TRUE(tr->Contains(6));
	ASSERT_TRUE(tr->TryRemove(6));
	ASSERT_FALSE(tr->Contains(6));
	ASSERT_FALSE(tr->TryRemove(6));
	ASSERT_FALSE(tr->Contains(6));
	ASSERT_TRUE(Enumerable::SequenceEqual(tr->Diff(), seq20En, comparers::Equals()));

	tr->Commit();
	tr.reset();

	// 22

	const auto seq21En = Enumerable::Take(Enumerable::Skip(seq14En, 1), 3);
	const auto seq21Ren = Enumerable::Take(Enumerable::Skip(seq14Ren, 1), 3);

	ASSERT_FALSE(s->IsEmpty());
	ASSERT_EQ(s->GetCount(), 3u);
	ASSERT_EQ(Enumerable::Count(s), 3u);

	ASSERT_TRUE(Enumerable::SequenceEqual(s, seq21En, comparers::Equals()));
	ASSERT_TRUE(Enumerable::SequenceEqual(s->Reverse(), seq21Ren, comparers::Equals()));

	ASSERT_TRUE(Enumerable::SequenceEqual(GetValueFromSignal(s->OnChanged()), WrapEnumerable(seq21En, Bind(&MakeDiffEntry<int>, CollectionOp::Added, _1)), comparers::Equals()));

	// 23
}
