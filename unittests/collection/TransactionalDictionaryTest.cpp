#include <stingraykit/collection/TransactionalDictionary.h>
#include <stingraykit/signal/ValueFromSignalObtainer.h>

#include <gtest/gtest.h>

#define CHECK_CONTAINS(Dict, Key, Value) \
		do { \
			ASSERT_TRUE(Dict->ContainsKey(Key)); \
			ASSERT_NO_THROW(Dict->Get(Key)); \
			ASSERT_EQ(Dict->Get(Key), Value); \
			decltype(Dict)::ValueType::ValueType value; \
			ASSERT_TRUE(Dict->TryGet(Key, value)); \
			ASSERT_EQ(value, Value); \
		} while(false)

#define CHECK_NOT_CONTAINS(Dict, Key) \
		do { \
			ASSERT_FALSE(Dict->ContainsKey(Key)); \
			ASSERT_ANY_THROW(Dict->Get(Key)); \
			decltype(Dict)::ValueType::ValueType value; \
			ASSERT_FALSE(Dict->TryGet(Key, value)); \
		} while(false)

using namespace stingray;

TEST(TransactionalDictionaryTest, Test2)
{
	using DictionaryType = ITransactionalDictionary<int, std::string>;
	using EntryType = DictionaryType::PairType;
	using EnumerableTypePtr = shared_ptr<IEnumerable<EntryType>>;

	// 1

	const shared_ptr<DictionaryType> s = make_shared_ptr<TransactionalDictionary<int, std::string>>();

	ASSERT_TRUE(s->IsEmpty());
	ASSERT_EQ(s->GetCount(), 0u);
	ASSERT_EQ(Enumerable::Count(s), 0u);

	ASSERT_TRUE(Enumerable::SequenceEqual(s, (EnumerableTypePtr)MakeEmptyEnumerable(), comparers::Equals()));
	ASSERT_TRUE(Enumerable::SequenceEqual(s->Reverse(), (EnumerableTypePtr)MakeEmptyEnumerable(), comparers::Equals()));

	ASSERT_FALSE(GetValueFromSignal(s->OnChanged()));

	// 2

	DictionaryType::TransactionTypePtr tr = s->StartTransaction();
	ASSERT_EQ(tr->GetCount(), 0u);
	ASSERT_TRUE(Enumerable::SequenceEqual(tr, (EnumerableTypePtr)MakeEmptyEnumerable(), comparers::Equals()));

	const EntryType seq1[] = {{1, "1"}, {2, "2"}, {3, "3"}, {4, "4"}, {5, "5"}};
	const auto seq1En = EnumerableFromStlIterators(std::begin(seq1), std::end(seq1));
	const auto seq1Ren = EnumerableFromStlIterators(std::rbegin(seq1), std::rend(seq1));

	CHECK_NOT_CONTAINS(tr, 1);
	tr->Set(1, "1");
	CHECK_CONTAINS(tr, 1, "1");
	ASSERT_EQ(tr->GetCount(), 1u);
	ASSERT_TRUE(Enumerable::SequenceEqual(tr, Enumerable::Take(seq1En, 1), comparers::Equals()));

	CHECK_NOT_CONTAINS(tr, 2);
	tr->Set(2, "2");
	CHECK_CONTAINS(tr, 2, "2");
	ASSERT_EQ(tr->GetCount(), 2u);
	ASSERT_TRUE(Enumerable::SequenceEqual(tr, Enumerable::Take(seq1En, 2), comparers::Equals()));

	CHECK_NOT_CONTAINS(tr, 3);
	tr->Set(3, "3");
	CHECK_CONTAINS(tr, 3, "3");
	ASSERT_EQ(tr->GetCount(), 3u);
	ASSERT_TRUE(Enumerable::SequenceEqual(tr, Enumerable::Take(seq1En, 3), comparers::Equals()));

	CHECK_NOT_CONTAINS(tr, 4);
	tr->Set(4, "4");
	CHECK_CONTAINS(tr, 4, "4");
	ASSERT_EQ(tr->GetCount(), 4u);
	ASSERT_TRUE(Enumerable::SequenceEqual(tr, Enumerable::Take(seq1En, 4), comparers::Equals()));

	CHECK_NOT_CONTAINS(tr, 5);
	tr->Set(5, "5");
	CHECK_CONTAINS(tr, 5, "5");
	ASSERT_EQ(tr->GetCount(), 5u);
	ASSERT_TRUE(Enumerable::SequenceEqual(tr, seq1En, comparers::Equals()));

	DictionaryType::DiffTypePtr diff1 = tr->Diff();

	CHECK_NOT_CONTAINS(tr, 6);
	ASSERT_FALSE(tr->Remove(6));
	CHECK_NOT_CONTAINS(tr, 6);
	ASSERT_EQ(tr->GetCount(), 5u);
	ASSERT_TRUE(Enumerable::SequenceEqual(tr, seq1En, comparers::Equals()));

	const EntryType seq2[] = {{1, "1"}, {3, "3"}, {4, "4"}, {5, "5"}};
	const auto seq2En = EnumerableFromStlIterators(std::begin(seq2), std::end(seq2));

	CHECK_CONTAINS(tr, 2, "2");
	ASSERT_TRUE(tr->Remove(2));
	CHECK_NOT_CONTAINS(tr, 2);
	ASSERT_EQ(tr->GetCount(), 4u);
	ASSERT_TRUE(Enumerable::SequenceEqual(tr, seq2En, comparers::Equals()));

	const EntryType seq3[] = {{1, "1"}, {3, "3"}, {5, "5"}};
	const auto seq3En = EnumerableFromStlIterators(std::begin(seq3), std::end(seq3));
	const auto seq3Ren = EnumerableFromStlIterators(std::rbegin(seq3), std::rend(seq3));

	CHECK_CONTAINS(tr, 4, "4");
	ASSERT_TRUE(tr->Remove(4));
	CHECK_NOT_CONTAINS(tr, 4);
	ASSERT_EQ(tr->GetCount(), 3u);
	ASSERT_TRUE(Enumerable::SequenceEqual(tr, seq3En, comparers::Equals()));

	DictionaryType::DiffTypePtr diff2 = tr->Diff();
	auto trEn1 = tr->GetEnumerator();

	tr->Commit();

	ASSERT_TRUE(Enumerable::SequenceEqual(diff1, WrapEnumerable(seq1En, Bind(&MakeDiffEntry<EntryType>, CollectionOp::Added, _1)), comparers::Equals()));
	ASSERT_TRUE(Enumerable::SequenceEqual(diff2, WrapEnumerable(seq3En, Bind(&MakeDiffEntry<EntryType>, CollectionOp::Added, _1)), comparers::Equals()));
	diff1.reset();
	diff2.reset();

	tr.reset();
	ASSERT_TRUE(Enumerable::SequenceEqual(trEn1, seq3En, comparers::Equals()));
	trEn1.reset();

	// 3

	ASSERT_FALSE(s->IsEmpty());
	ASSERT_EQ(s->GetCount(), 3u);
	ASSERT_EQ(Enumerable::Count(s), 3u);

	ASSERT_TRUE(Enumerable::SequenceEqual(s, seq3En, comparers::Equals()));
	ASSERT_TRUE(Enumerable::SequenceEqual(s->Reverse(), seq3Ren, comparers::Equals()));

	ASSERT_TRUE(Enumerable::SequenceEqual(GetValueFromSignal(s->OnChanged()), WrapEnumerable(seq3En, Bind(&MakeDiffEntry<EntryType>, CollectionOp::Added, _1)), comparers::Equals()));

	// 4

	tr = s->StartTransaction();
	ASSERT_EQ(tr->GetCount(), 3u);
	ASSERT_TRUE(Enumerable::SequenceEqual(tr, seq3En, comparers::Equals()));

	const EntryType seq4[] = {{1, "1"}, {2, "2"}, {3, "3"}, {5, "5"}};
	const auto seq4En = EnumerableFromStlIterators(std::begin(seq4), std::end(seq4));

	CHECK_NOT_CONTAINS(tr, 2);
	tr->Set(2, "2");
	CHECK_CONTAINS(tr, 2, "2");
	ASSERT_EQ(tr->GetCount(), 4u);
	ASSERT_TRUE(Enumerable::SequenceEqual(tr, seq4En, comparers::Equals()));

	CHECK_NOT_CONTAINS(tr, 4);
	tr->Set(4, "4");
	CHECK_CONTAINS(tr, 4, "4");
	ASSERT_EQ(tr->GetCount(), 5u);
	ASSERT_TRUE(Enumerable::SequenceEqual(tr, seq1En, comparers::Equals()));

	diff1 = tr->Diff();

	const EntryType seq5[] = {{2, "2"}, {3, "3"}, {4, "4"}, {5, "5"}};
	const auto seq5En = EnumerableFromStlIterators(std::begin(seq5), std::end(seq5));

	CHECK_CONTAINS(tr, 1, "1");
	ASSERT_TRUE(tr->Remove(1));
	CHECK_NOT_CONTAINS(tr, 1);
	ASSERT_EQ(tr->GetCount(), 4u);
	ASSERT_TRUE(Enumerable::SequenceEqual(tr, seq5En, comparers::Equals()));

	const EntryType seq6[] = {{2, "2"}, {4, "4"}, {5, "5"}};
	const auto seq6En = EnumerableFromStlIterators(std::begin(seq6), std::end(seq6));

	CHECK_CONTAINS(tr, 3, "3");
	ASSERT_TRUE(tr->Remove(3));
	CHECK_NOT_CONTAINS(tr, 3);
	ASSERT_EQ(tr->GetCount(), 3u);
	ASSERT_TRUE(Enumerable::SequenceEqual(tr, seq6En, comparers::Equals()));

	CHECK_NOT_CONTAINS(tr, 3);
	ASSERT_FALSE(tr->Remove(3));
	CHECK_NOT_CONTAINS(tr, 3);
	ASSERT_EQ(tr->GetCount(), 3u);
	ASSERT_TRUE(Enumerable::SequenceEqual(tr, seq6En, comparers::Equals()));

	const EntryType seq7[] = {{2, "2"}, {4, "4"}};
	const auto seq7En = EnumerableFromStlIterators(std::begin(seq7), std::end(seq7));
	const auto seq7Ren = EnumerableFromStlIterators(std::rbegin(seq7), std::rend(seq7));

	CHECK_CONTAINS(tr, 5, "5");
	ASSERT_TRUE(tr->Remove(5));
	CHECK_NOT_CONTAINS(tr, 5);
	ASSERT_EQ(tr->GetCount(), 2u);
	ASSERT_TRUE(Enumerable::SequenceEqual(tr, seq7En, comparers::Equals()));

	CHECK_NOT_CONTAINS(tr, 5);
	ASSERT_FALSE(tr->Remove(5));
	CHECK_NOT_CONTAINS(tr, 5);
	ASSERT_EQ(tr->GetCount(), 2u);
	ASSERT_TRUE(Enumerable::SequenceEqual(tr, seq7En, comparers::Equals()));

	diff2 = tr->Diff();

	trEn1 = tr->GetEnumerator();
	tr->Commit();

	const DiffEntry<EntryType> seq8[] = {{CollectionOp::Removed, {1, "1"}}, {CollectionOp::Added, {2, "2"}}, {CollectionOp::Removed, {3, "3"}}, {CollectionOp::Added, {4, "4"}}, {CollectionOp::Removed, {5, "5"}}};
	const auto seq8En = EnumerableFromStlIterators(std::begin(seq8), std::end(seq8));

	ASSERT_TRUE(Enumerable::SequenceEqual(diff1, WrapEnumerable(seq7En, Bind(&MakeDiffEntry<EntryType>, CollectionOp::Added, _1)), comparers::Equals()));
	ASSERT_TRUE(Enumerable::SequenceEqual(diff2, seq8En, comparers::Equals()));
	diff1.reset();
	diff2.reset();

	tr.reset();
	ASSERT_TRUE(Enumerable::SequenceEqual(trEn1, seq7En, comparers::Equals()));
	trEn1.reset();

	// 5

	ASSERT_FALSE(s->IsEmpty());
	ASSERT_EQ(s->GetCount(), 2u);
	ASSERT_EQ(Enumerable::Count(s), 2u);

	ASSERT_TRUE(Enumerable::SequenceEqual(s, seq7En, comparers::Equals()));
	ASSERT_TRUE(Enumerable::SequenceEqual(s->Reverse(), seq7Ren, comparers::Equals()));

	ASSERT_TRUE(Enumerable::SequenceEqual(GetValueFromSignal(s->OnChanged()), WrapEnumerable(seq7En, Bind(&MakeDiffEntry<EntryType>, CollectionOp::Added, _1)), comparers::Equals()));

	// 6

	tr = s->StartTransaction();
	ASSERT_TRUE(Enumerable::SequenceEqual(tr, seq7En, comparers::Equals()));

	const EntryType seq9[] = {{1, "1"}, {2, "2"}, {4, "4"}};
	const auto seq9En = EnumerableFromStlIterators(std::begin(seq9), std::end(seq9));

	CHECK_NOT_CONTAINS(tr, 1);
	tr->Set(1, "1");
	CHECK_CONTAINS(tr, 1, "1");
	ASSERT_TRUE(Enumerable::SequenceEqual(tr, seq9En, comparers::Equals()));

	CHECK_CONTAINS(tr, 2, "2");
	tr->Set(2, "2");
	CHECK_CONTAINS(tr, 2, "2");
	ASSERT_TRUE(Enumerable::SequenceEqual(tr, seq9En, comparers::Equals()));

	const EntryType seq10[] = {{1, "1"}, {2, "2"}, {3, "3"}, {4, "4"}};
	const auto seq10En = EnumerableFromStlIterators(std::begin(seq10), std::end(seq10));

	CHECK_NOT_CONTAINS(tr, 3);
	tr->Set(3, "3");
	CHECK_CONTAINS(tr, 3, "3");
	ASSERT_TRUE(Enumerable::SequenceEqual(tr, seq10En, comparers::Equals()));

	CHECK_CONTAINS(tr, 4, "4");
	tr->Set(4, "4");
	CHECK_CONTAINS(tr, 4, "4");
	ASSERT_TRUE(Enumerable::SequenceEqual(tr, seq10En, comparers::Equals()));

	CHECK_NOT_CONTAINS(tr, 5);
	tr->Set(5, "5");
	CHECK_CONTAINS(tr, 5, "5");
	ASSERT_TRUE(Enumerable::SequenceEqual(tr, seq1En, comparers::Equals()));

	diff1 = tr->Diff();

	trEn1 = tr->GetEnumerator();
	tr->Commit();

	ASSERT_TRUE(Enumerable::SequenceEqual(diff1, WrapEnumerable(seq3En, Bind(&MakeDiffEntry<EntryType>, CollectionOp::Added, _1)), comparers::Equals()));
	diff1.reset();

	tr.reset();
	ASSERT_TRUE(Enumerable::SequenceEqual(trEn1, seq1En, comparers::Equals()));
	trEn1.reset();

	// 7

	ASSERT_FALSE(s->IsEmpty());
	ASSERT_EQ(s->GetCount(), 5u);
	ASSERT_EQ(Enumerable::Count(s), 5u);

	ASSERT_TRUE(Enumerable::SequenceEqual(s, seq1En, comparers::Equals()));
	ASSERT_TRUE(Enumerable::SequenceEqual(s->Reverse(), seq1Ren, comparers::Equals()));

	ASSERT_TRUE(Enumerable::SequenceEqual(GetValueFromSignal(s->OnChanged()), WrapEnumerable(seq1En, Bind(&MakeDiffEntry<EntryType>, CollectionOp::Added, _1)), comparers::Equals()));

	// 8

	ASSERT_TRUE(Enumerable::SequenceEqual(s->Find(0), (EnumerableTypePtr)MakeEmptyEnumerable(), comparers::Equals()));
	ASSERT_TRUE(Enumerable::SequenceEqual(s->Find(1), seq1En, comparers::Equals()));
	ASSERT_TRUE(Enumerable::SequenceEqual(s->Find(2), Enumerable::Skip(seq1En, 1), comparers::Equals()));
	ASSERT_TRUE(Enumerable::SequenceEqual(s->Find(3), Enumerable::Skip(seq1En, 2), comparers::Equals()));
	ASSERT_TRUE(Enumerable::SequenceEqual(s->Find(4), Enumerable::Skip(seq1En, 3), comparers::Equals()));
	ASSERT_TRUE(Enumerable::SequenceEqual(s->Find(5), Enumerable::Skip(seq1En, 4), comparers::Equals()));
	ASSERT_TRUE(Enumerable::SequenceEqual(s->Find(6), (EnumerableTypePtr)MakeEmptyEnumerable(), comparers::Equals()));

	ASSERT_TRUE(Enumerable::SequenceEqual(s->ReverseFind(0), (EnumerableTypePtr)MakeEmptyEnumerable(), comparers::Equals()));
	ASSERT_TRUE(Enumerable::SequenceEqual(s->ReverseFind(1), Enumerable::Skip(seq1Ren, 4), comparers::Equals()));
	ASSERT_TRUE(Enumerable::SequenceEqual(s->ReverseFind(2), Enumerable::Skip(seq1Ren, 3), comparers::Equals()));
	ASSERT_TRUE(Enumerable::SequenceEqual(s->ReverseFind(3), Enumerable::Skip(seq1Ren, 2), comparers::Equals()));
	ASSERT_TRUE(Enumerable::SequenceEqual(s->ReverseFind(4), Enumerable::Skip(seq1Ren, 1), comparers::Equals()));
	ASSERT_TRUE(Enumerable::SequenceEqual(s->ReverseFind(5), seq1Ren, comparers::Equals()));
	ASSERT_TRUE(Enumerable::SequenceEqual(s->ReverseFind(6), (EnumerableTypePtr)MakeEmptyEnumerable(), comparers::Equals()));

	// 9

	tr = s->StartTransaction();
	ASSERT_TRUE(Enumerable::SequenceEqual(tr, seq1En, comparers::Equals()));
	ASSERT_TRUE(Enumerable::SequenceEqual(tr->Reverse(), seq1Ren, comparers::Equals()));

	const EntryType seq11[] = {{0, "0"}, {1, "1"}, {2, "2"}, {3, "3"}, {4, "4"}, {5, "5"}};
	const auto seq11En = EnumerableFromStlIterators(std::begin(seq11), std::end(seq11));
	const auto seq11Ren = EnumerableFromStlIterators(std::rbegin(seq11), std::rend(seq11));

	CHECK_NOT_CONTAINS(tr, 0);
	tr->Set(0, "0");
	CHECK_CONTAINS(tr, 0, "0");
	ASSERT_TRUE(Enumerable::SequenceEqual(tr, seq11En, comparers::Equals()));
	ASSERT_TRUE(Enumerable::SequenceEqual(tr->Reverse(), seq11Ren, comparers::Equals()));

	const EntryType seq12[] = {{0, "0"}, {1, "1"}, {3, "3"}, {4, "4"}, {5, "5"}};
	const auto seq12En = EnumerableFromStlIterators(std::begin(seq12), std::end(seq12));
	const auto seq12Ren = EnumerableFromStlIterators(std::rbegin(seq12), std::rend(seq12));

	CHECK_CONTAINS(tr, 2, "2");
	ASSERT_TRUE(tr->Remove(2));
	CHECK_NOT_CONTAINS(tr, 2);
	ASSERT_TRUE(Enumerable::SequenceEqual(tr, seq12En, comparers::Equals()));
	ASSERT_TRUE(Enumerable::SequenceEqual(tr->Reverse(), seq12Ren, comparers::Equals()));

	const EntryType seq13[] = {{0, "0"}, {1, "1"}, {3, "3"}, {5, "5"}};
	const auto seq13En = EnumerableFromStlIterators(std::begin(seq13), std::end(seq13));
	const auto seq13Ren = EnumerableFromStlIterators(std::rbegin(seq13), std::rend(seq13));

	CHECK_CONTAINS(tr, 4, "4");
	ASSERT_TRUE(tr->Remove(4));
	CHECK_NOT_CONTAINS(tr, 4);
	ASSERT_TRUE(Enumerable::SequenceEqual(tr, seq13En, comparers::Equals()));
	ASSERT_TRUE(Enumerable::SequenceEqual(tr->Reverse(), seq13Ren, comparers::Equals()));

	const EntryType seq14[] = {{0, "0"}, {1, "1"}, {3, "3"}, {5, "5"}, {6, "6"}};
	const auto seq14En = EnumerableFromStlIterators(std::begin(seq14), std::end(seq14));
	const auto seq14Ren = EnumerableFromStlIterators(std::rbegin(seq14), std::rend(seq14));

	CHECK_NOT_CONTAINS(tr, 6);
	tr->Set(6, "6");
	CHECK_CONTAINS(tr, 6, "6");
	ASSERT_TRUE(Enumerable::SequenceEqual(tr, seq14En, comparers::Equals()));
	ASSERT_TRUE(Enumerable::SequenceEqual(tr->Reverse(), seq14Ren, comparers::Equals()));

	diff1 = tr->Diff();

	// 10

	ASSERT_TRUE(Enumerable::SequenceEqual(tr->Find(-1), (EnumerableTypePtr)MakeEmptyEnumerable(), comparers::Equals()));
	ASSERT_TRUE(Enumerable::SequenceEqual(tr->Find(0), seq14En, comparers::Equals()));
	ASSERT_TRUE(Enumerable::SequenceEqual(tr->Find(1), Enumerable::Skip(seq14En, 1), comparers::Equals()));
	ASSERT_TRUE(Enumerable::SequenceEqual(tr->Find(2), (EnumerableTypePtr)MakeEmptyEnumerable(), comparers::Equals()));
	ASSERT_TRUE(Enumerable::SequenceEqual(tr->Find(3), Enumerable::Skip(seq14En, 2), comparers::Equals()));
	ASSERT_TRUE(Enumerable::SequenceEqual(tr->Find(4), (EnumerableTypePtr)MakeEmptyEnumerable(), comparers::Equals()));
	ASSERT_TRUE(Enumerable::SequenceEqual(tr->Find(5), Enumerable::Skip(seq14En, 3), comparers::Equals()));
	ASSERT_TRUE(Enumerable::SequenceEqual(tr->Find(6), Enumerable::Skip(seq14En, 4), comparers::Equals()));
	ASSERT_TRUE(Enumerable::SequenceEqual(tr->Find(7), (EnumerableTypePtr)MakeEmptyEnumerable(), comparers::Equals()));

	ASSERT_TRUE(Enumerable::SequenceEqual(tr->ReverseFind(-1), (EnumerableTypePtr)MakeEmptyEnumerable(), comparers::Equals()));
	ASSERT_TRUE(Enumerable::SequenceEqual(tr->ReverseFind(0), Enumerable::Skip(seq14Ren, 4), comparers::Equals()));
	ASSERT_TRUE(Enumerable::SequenceEqual(tr->ReverseFind(1), Enumerable::Skip(seq14Ren, 3), comparers::Equals()));
	ASSERT_TRUE(Enumerable::SequenceEqual(tr->ReverseFind(2), (EnumerableTypePtr)MakeEmptyEnumerable(), comparers::Equals()));
	ASSERT_TRUE(Enumerable::SequenceEqual(tr->ReverseFind(3), Enumerable::Skip(seq14Ren, 2), comparers::Equals()));
	ASSERT_TRUE(Enumerable::SequenceEqual(tr->ReverseFind(4), (EnumerableTypePtr)MakeEmptyEnumerable(), comparers::Equals()));
	ASSERT_TRUE(Enumerable::SequenceEqual(tr->ReverseFind(5), Enumerable::Skip(seq14Ren, 1), comparers::Equals()));
	ASSERT_TRUE(Enumerable::SequenceEqual(tr->ReverseFind(6), seq14Ren, comparers::Equals()));
	ASSERT_TRUE(Enumerable::SequenceEqual(tr->ReverseFind(7), (EnumerableTypePtr)MakeEmptyEnumerable(), comparers::Equals()));

	// 11

	tr->Commit();
	tr.reset();

	const DiffEntry<EntryType> seq15[] = {{CollectionOp::Added, {0, "0"}}, {CollectionOp::Removed, {2, "2"}}, {CollectionOp::Removed, {4, "4"}}, {CollectionOp::Added, {6, "6"}}};
	const auto seq15En = EnumerableFromStlIterators(std::begin(seq15), std::end(seq15));

	ASSERT_TRUE(Enumerable::SequenceEqual(diff1, seq15En, comparers::Equals()));
	diff1.reset();

	// 12

	ASSERT_FALSE(s->IsEmpty());
	ASSERT_EQ(s->GetCount(), 5u);
	ASSERT_EQ(Enumerable::Count(s), 5u);

	ASSERT_TRUE(Enumerable::SequenceEqual(s, seq14En, comparers::Equals()));
	ASSERT_TRUE(Enumerable::SequenceEqual(s->Reverse(), seq14Ren, comparers::Equals()));

	ASSERT_TRUE(Enumerable::SequenceEqual(GetValueFromSignal(s->OnChanged()), WrapEnumerable(seq14En, Bind(&MakeDiffEntry<EntryType>, CollectionOp::Added, _1)), comparers::Equals()));

	// 13

	tr = s->StartTransaction();
	ASSERT_TRUE(Enumerable::SequenceEqual(tr, seq14En, comparers::Equals()));
	ASSERT_TRUE(Enumerable::SequenceEqual(tr->Reverse(), seq14Ren, comparers::Equals()));

	CHECK_CONTAINS(tr, 1, "1");
	ASSERT_TRUE(tr->Remove(1));
	CHECK_NOT_CONTAINS(tr, 1);

	CHECK_CONTAINS(tr, 3, "3");
	ASSERT_TRUE(tr->Remove(3));
	CHECK_NOT_CONTAINS(tr, 3);

	CHECK_CONTAINS(tr, 5, "5");
	ASSERT_TRUE(tr->Remove(5));
	CHECK_NOT_CONTAINS(tr, 5);

	const DiffEntry<EntryType> seq16[] = {{CollectionOp::Removed, {1, "1"}}, {CollectionOp::Removed, {3, "3"}}, {CollectionOp::Removed, {5, "5"}}};
	const auto seq16En = EnumerableFromStlIterators(std::begin(seq16), std::end(seq16));

	diff1 = tr->Diff();
	tr->Revert();
	diff2 = tr->Diff();
	tr.reset();

	ASSERT_TRUE(Enumerable::SequenceEqual(diff1, seq16En, comparers::Equals()));
	diff1.reset();

	ASSERT_TRUE(Enumerable::SequenceEqual(diff2, (DictionaryType::DiffTypePtr)MakeEmptyEnumerable(), comparers::Equals()));
	diff2.reset();

	// 14

	ASSERT_FALSE(s->IsEmpty());
	ASSERT_EQ(s->GetCount(), 5u);
	ASSERT_EQ(Enumerable::Count(s), 5u);

	ASSERT_TRUE(Enumerable::SequenceEqual(s, seq14En, comparers::Equals()));
	ASSERT_TRUE(Enumerable::SequenceEqual(s->Reverse(), seq14Ren, comparers::Equals()));

	ASSERT_TRUE(Enumerable::SequenceEqual(GetValueFromSignal(s->OnChanged()), WrapEnumerable(seq14En, Bind(&MakeDiffEntry<EntryType>, CollectionOp::Added, _1)), comparers::Equals()));

	// 15

	tr = s->StartTransaction();
	ASSERT_EQ(tr->GetCount(), 5u);
	ASSERT_TRUE(Enumerable::SequenceEqual(tr, seq14En, comparers::Equals()));
	ASSERT_TRUE(Enumerable::SequenceEqual(tr->Reverse(), seq14Ren, comparers::Equals()));

	const DiffEntry<EntryType> seq17[] = {{CollectionOp::Removed, {0, "0"}}, {CollectionOp::Added, {0, "00"}}, {CollectionOp::Removed, {3, "3"}}, {CollectionOp::Added, {3, "33"}}, {CollectionOp::Removed, {6, "6"}}, {CollectionOp::Added, {6, "66"}}};
	const auto seq17En = EnumerableFromStlIterators(std::begin(seq17), std::end(seq17));

	CHECK_CONTAINS(tr, 0, "0");
	tr->Set(0, "00");
	CHECK_CONTAINS(tr, 0, "00");
	ASSERT_EQ(tr->GetCount(), 5u);
	ASSERT_TRUE(Enumerable::SequenceEqual(tr->Diff(), Enumerable::Take(seq17En, 2), comparers::Equals()));

	CHECK_CONTAINS(tr, 3, "3");
	tr->Set(3, "33");
	CHECK_CONTAINS(tr, 3, "33");
	ASSERT_EQ(tr->GetCount(), 5u);
	ASSERT_TRUE(Enumerable::SequenceEqual(tr->Diff(), Enumerable::Take(seq17En, 4), comparers::Equals()));

	CHECK_CONTAINS(tr, 6, "6");
	tr->Set(6, "66");
	CHECK_CONTAINS(tr, 6, "66");
	ASSERT_EQ(tr->GetCount(), 5u);
	ASSERT_TRUE(Enumerable::SequenceEqual(tr->Diff(), seq17En, comparers::Equals()));

	CHECK_CONTAINS(tr, 0, "00");
	tr->Set(0, "0");
	CHECK_CONTAINS(tr, 0, "0");
	ASSERT_EQ(tr->GetCount(), 5u);
	ASSERT_TRUE(Enumerable::SequenceEqual(tr->Diff(), Enumerable::Skip(seq17En, 2), comparers::Equals()));

	CHECK_CONTAINS(tr, 3, "33");
	tr->Set(3, "3");
	CHECK_CONTAINS(tr, 3, "3");
	ASSERT_EQ(tr->GetCount(), 5u);
	ASSERT_TRUE(Enumerable::SequenceEqual(tr->Diff(), Enumerable::Skip(seq17En, 4), comparers::Equals()));

	CHECK_CONTAINS(tr, 6, "66");
	tr->Set(6, "6");
	CHECK_CONTAINS(tr, 6, "6");
	ASSERT_EQ(tr->GetCount(), 5u);
	ASSERT_TRUE(Enumerable::SequenceEqual(tr->Diff(), (DictionaryType::DiffTypePtr)MakeEmptyEnumerable(), comparers::Equals()));

	tr->Commit();
	tr.reset();

	// 16

	ASSERT_FALSE(s->IsEmpty());
	ASSERT_EQ(s->GetCount(), 5u);
	ASSERT_EQ(Enumerable::Count(s), 5u);

	ASSERT_TRUE(Enumerable::SequenceEqual(s, seq14En, comparers::Equals()));
	ASSERT_TRUE(Enumerable::SequenceEqual(s->Reverse(), seq14Ren, comparers::Equals()));

	ASSERT_TRUE(Enumerable::SequenceEqual(GetValueFromSignal(s->OnChanged()), WrapEnumerable(seq14En, Bind(&MakeDiffEntry<EntryType>, CollectionOp::Added, _1)), comparers::Equals()));

	// 17

	tr = s->StartTransaction();
	ASSERT_EQ(tr->GetCount(), 5u);
	ASSERT_TRUE(Enumerable::SequenceEqual(tr, seq14En, comparers::Equals()));
	ASSERT_TRUE(Enumerable::SequenceEqual(tr->Reverse(), seq14Ren, comparers::Equals()));

	CHECK_CONTAINS(tr, 6, "6");
	tr->Set(6, "66");
	CHECK_CONTAINS(tr, 6, "66");
	ASSERT_EQ(tr->GetCount(), 5u);
	ASSERT_TRUE(Enumerable::SequenceEqual(tr->Diff(), Enumerable::Skip(seq17En, 4), comparers::Equals()));

	CHECK_CONTAINS(tr, 3, "3");
	tr->Set(3, "33");
	CHECK_CONTAINS(tr, 3, "33");
	ASSERT_EQ(tr->GetCount(), 5u);
	ASSERT_TRUE(Enumerable::SequenceEqual(tr->Diff(), Enumerable::Skip(seq17En, 2), comparers::Equals()));

	CHECK_CONTAINS(tr, 0, "0");
	tr->Set(0, "00");
	CHECK_CONTAINS(tr, 0, "00");
	ASSERT_EQ(tr->GetCount(), 5u);
	ASSERT_TRUE(Enumerable::SequenceEqual(tr->Diff(), seq17En, comparers::Equals()));

	tr->Commit();
	tr.reset();

	// 18

	const EntryType seq18[] = {{0, "00"}, {1, "1"}, {3, "33"}, {5, "5"}, {6, "66"}};
	const auto seq18En = EnumerableFromStlIterators(std::begin(seq18), std::end(seq18));
	const auto seq18Ren = EnumerableFromStlIterators(std::rbegin(seq18), std::rend(seq18));

	ASSERT_FALSE(s->IsEmpty());
	ASSERT_EQ(s->GetCount(), 5u);
	ASSERT_EQ(Enumerable::Count(s), 5u);

	ASSERT_TRUE(Enumerable::SequenceEqual(s, seq18En, comparers::Equals()));
	ASSERT_TRUE(Enumerable::SequenceEqual(s->Reverse(), seq18Ren, comparers::Equals()));

	ASSERT_TRUE(Enumerable::SequenceEqual(GetValueFromSignal(s->OnChanged()), WrapEnumerable(seq18En, Bind(&MakeDiffEntry<EntryType>, CollectionOp::Added, _1)), comparers::Equals()));

	// 19

	tr = s->StartTransaction();
	ASSERT_TRUE(Enumerable::SequenceEqual(tr, seq18En, comparers::Equals()));
	ASSERT_TRUE(Enumerable::SequenceEqual(tr->Reverse(), seq18Ren, comparers::Equals()));

	const DiffEntry<EntryType> seq19[] = {{CollectionOp::Added, {2, "2"}}, {CollectionOp::Added, {4, "4"}}};
	const auto seq19En = EnumerableFromStlIterators(std::begin(seq19), std::end(seq19));

	CHECK_NOT_CONTAINS(tr, 2);
	tr->Set(2, "2");
	CHECK_CONTAINS(tr, 2, "2");
	ASSERT_EQ(tr->GetCount(), 6u);
	ASSERT_TRUE(Enumerable::SequenceEqual(tr->Diff(), Enumerable::Take(seq19En, 1), comparers::Equals()));

	CHECK_NOT_CONTAINS(tr, 4);
	tr->Set(4, "4");
	CHECK_CONTAINS(tr, 4, "4");
	ASSERT_EQ(tr->GetCount(), 7u);
	ASSERT_TRUE(Enumerable::SequenceEqual(tr->Diff(), seq19En, comparers::Equals()));

	CHECK_CONTAINS(tr, 2, "2");
	ASSERT_TRUE(tr->TryRemove(2));
	CHECK_NOT_CONTAINS(tr, 2);
	ASSERT_FALSE(tr->TryRemove(2));
	CHECK_NOT_CONTAINS(tr, 2);
	ASSERT_EQ(tr->GetCount(), 6u);
	ASSERT_TRUE(Enumerable::SequenceEqual(tr->Diff(), Enumerable::Skip(seq19En, 1), comparers::Equals()));

	CHECK_CONTAINS(tr, 4, "4");
	ASSERT_TRUE(tr->TryRemove(4));
	CHECK_NOT_CONTAINS(tr, 4);
	ASSERT_FALSE(tr->TryRemove(4));
	CHECK_NOT_CONTAINS(tr, 4);
	ASSERT_EQ(tr->GetCount(), 5u);
	ASSERT_TRUE(Enumerable::SequenceEqual(tr->Diff(), (DictionaryType::DiffTypePtr)MakeEmptyEnumerable(), comparers::Equals()));

	tr->Commit();
	tr.reset();

	// 20

	ASSERT_FALSE(s->IsEmpty());
	ASSERT_EQ(s->GetCount(), 5u);
	ASSERT_EQ(Enumerable::Count(s), 5u);

	ASSERT_TRUE(Enumerable::SequenceEqual(s, seq18En, comparers::Equals()));
	ASSERT_TRUE(Enumerable::SequenceEqual(s->Reverse(), seq18Ren, comparers::Equals()));

	ASSERT_TRUE(Enumerable::SequenceEqual(GetValueFromSignal(s->OnChanged()), WrapEnumerable(seq18En, Bind(&MakeDiffEntry<EntryType>, CollectionOp::Added, _1)), comparers::Equals()));

	// 21

	tr = s->StartTransaction();
	ASSERT_TRUE(Enumerable::SequenceEqual(tr, seq18En, comparers::Equals()));
	ASSERT_TRUE(Enumerable::SequenceEqual(tr->Reverse(), seq18Ren, comparers::Equals()));

	const DiffEntry<EntryType> seq20[] = {{CollectionOp::Removed, {0, "00"}}, {CollectionOp::Removed, {6, "66"}}};
	const auto seq20En = EnumerableFromStlIterators(std::begin(seq20), std::end(seq20));

	CHECK_CONTAINS(tr, 0, "00");
	ASSERT_TRUE(tr->TryRemove(0));
	CHECK_NOT_CONTAINS(tr, 0);
	ASSERT_FALSE(tr->TryRemove(0));
	CHECK_NOT_CONTAINS(tr, 0);
	ASSERT_TRUE(Enumerable::SequenceEqual(tr->Diff(), Enumerable::Take(seq20En, 1), comparers::Equals()));

	CHECK_CONTAINS(tr, 6, "66");
	ASSERT_TRUE(tr->TryRemove(6));
	CHECK_NOT_CONTAINS(tr, 6);
	ASSERT_FALSE(tr->TryRemove(6));
	CHECK_NOT_CONTAINS(tr, 6);
	ASSERT_TRUE(Enumerable::SequenceEqual(tr->Diff(), seq20En, comparers::Equals()));

	tr->Commit();
	tr.reset();

	// 22

	const auto seq21En = Enumerable::Take(Enumerable::Skip(seq18En, 1), 3);
	const auto seq21Ren = Enumerable::Take(Enumerable::Skip(seq18Ren, 1), 3);

	ASSERT_FALSE(s->IsEmpty());
	ASSERT_EQ(s->GetCount(), 3u);
	ASSERT_EQ(Enumerable::Count(s), 3u);

	ASSERT_TRUE(Enumerable::SequenceEqual(s, seq21En, comparers::Equals()));
	ASSERT_TRUE(Enumerable::SequenceEqual(s->Reverse(), seq21Ren, comparers::Equals()));

	ASSERT_TRUE(Enumerable::SequenceEqual(GetValueFromSignal(s->OnChanged()), WrapEnumerable(seq21En, Bind(&MakeDiffEntry<EntryType>, CollectionOp::Added, _1)), comparers::Equals()));

	// 23

	tr = s->StartTransaction();
	ASSERT_TRUE(Enumerable::SequenceEqual(tr, seq21En, comparers::Equals()));
	ASSERT_TRUE(Enumerable::SequenceEqual(tr->Reverse(), seq21Ren, comparers::Equals()));

	diff1 = tr->Diff();

	const EntryType seq22[] = {{0, "0"}, {1, "1"}, {3, "33"}, {5, "5"}};
	const auto seq22En = EnumerableFromStlIterators(std::begin(seq22), std::end(seq22));
	const auto seq22Ren = EnumerableFromStlIterators(std::rbegin(seq22), std::rend(seq22));

	CHECK_NOT_CONTAINS(tr, 0);
	tr->Set(0, "0");
	CHECK_CONTAINS(tr, 0, "0");
	ASSERT_TRUE(Enumerable::SequenceEqual(tr, seq22En, comparers::Equals()));
	ASSERT_TRUE(Enumerable::SequenceEqual(tr->Reverse(), seq22Ren, comparers::Equals()));

	const DiffEntry<EntryType> seq23[] = {{CollectionOp::Added, {0, "0"}}};
	const auto seq23En = EnumerableFromStlIterators(std::begin(seq23), std::end(seq23));

	ASSERT_TRUE(Enumerable::SequenceEqual(diff1, (DictionaryType::DiffTypePtr)MakeEmptyEnumerable(), comparers::Equals()));
	diff1 = tr->Diff();

	CHECK_CONTAINS(tr, 0, "0");
	tr->Set(0, "0");
	CHECK_CONTAINS(tr, 0, "0");
	ASSERT_TRUE(Enumerable::SequenceEqual(tr, seq22En, comparers::Equals()));
	ASSERT_TRUE(Enumerable::SequenceEqual(tr->Reverse(), seq22Ren, comparers::Equals()));

	ASSERT_TRUE(Enumerable::SequenceEqual(diff1, seq23En, comparers::Equals()));
	diff1 = tr->Diff();

	const EntryType seq24[] = {{0, "00"}, {1, "1"}, {3, "33"}, {5, "5"}};
	const auto seq24En = EnumerableFromStlIterators(std::begin(seq24), std::end(seq24));
	const auto seq24Ren = EnumerableFromStlIterators(std::rbegin(seq24), std::rend(seq24));

	CHECK_CONTAINS(tr, 0, "0");
	tr->Set(0, "00");
	CHECK_CONTAINS(tr, 0, "00");
	ASSERT_TRUE(Enumerable::SequenceEqual(tr, seq24En, comparers::Equals()));
	ASSERT_TRUE(Enumerable::SequenceEqual(tr->Reverse(), seq24Ren, comparers::Equals()));

	const DiffEntry<EntryType> seq25[] = {{CollectionOp::Added, {0, "00"}}};
	const auto seq25En = EnumerableFromStlIterators(std::begin(seq25), std::end(seq25));

	ASSERT_TRUE(Enumerable::SequenceEqual(diff1, seq23En, comparers::Equals()));
	diff1 = tr->Diff();

	const EntryType seq26[] = {{0, "00"}, {1, "1"}, {3, "3"}, {5, "5"}};
	const auto seq26En = EnumerableFromStlIterators(std::begin(seq26), std::end(seq26));
	const auto seq26Ren = EnumerableFromStlIterators(std::rbegin(seq26), std::rend(seq26));

	CHECK_CONTAINS(tr, 3, "33");
	tr->Set(3, "3");
	CHECK_CONTAINS(tr, 3, "3");
	ASSERT_TRUE(Enumerable::SequenceEqual(tr, seq26En, comparers::Equals()));
	ASSERT_TRUE(Enumerable::SequenceEqual(tr->Reverse(), seq26Ren, comparers::Equals()));

	const DiffEntry<EntryType> seq27[] = {{CollectionOp::Added, {0, "00"}}, {CollectionOp::Removed, {3, "33"}}, {CollectionOp::Added, {3, "3"}}};
	const auto seq27En = EnumerableFromStlIterators(std::begin(seq27), std::end(seq27));
	const auto seq27Ren = EnumerableFromStlIterators(std::rbegin(seq27), std::rend(seq27));

	ASSERT_TRUE(Enumerable::SequenceEqual(diff1, seq25En, comparers::Equals()));
	diff1 = tr->Diff();

	CHECK_CONTAINS(tr, 3, "3");
	tr->Set(3, "3");
	CHECK_CONTAINS(tr, 3, "3");
	ASSERT_TRUE(Enumerable::SequenceEqual(tr, seq26En, comparers::Equals()));
	ASSERT_TRUE(Enumerable::SequenceEqual(tr->Reverse(), seq26Ren, comparers::Equals()));

	ASSERT_TRUE(Enumerable::SequenceEqual(diff1, seq27En, comparers::Equals()));
	diff1 = tr->Diff();

	CHECK_CONTAINS(tr, 3, "3");
	tr->Set(3, "33");
	CHECK_CONTAINS(tr, 3, "33");
	ASSERT_TRUE(Enumerable::SequenceEqual(tr, seq24En, comparers::Equals()));
	ASSERT_TRUE(Enumerable::SequenceEqual(tr->Reverse(), seq24Ren, comparers::Equals()));

	ASSERT_TRUE(Enumerable::SequenceEqual(diff1, seq27En, comparers::Equals()));
	diff1 = tr->Diff();

	CHECK_CONTAINS(tr, 0, "00");
	ASSERT_TRUE(tr->TryRemove(0));
	CHECK_NOT_CONTAINS(tr, 0);
	ASSERT_TRUE(Enumerable::SequenceEqual(tr, seq21En, comparers::Equals()));
	ASSERT_TRUE(Enumerable::SequenceEqual(tr->Reverse(), seq21Ren, comparers::Equals()));

	ASSERT_TRUE(Enumerable::SequenceEqual(diff1, seq25En, comparers::Equals()));
	diff1 = tr->Diff();

	CHECK_NOT_CONTAINS(tr, 0);
	ASSERT_FALSE(tr->TryRemove(0));
	CHECK_NOT_CONTAINS(tr, 0);
	ASSERT_TRUE(Enumerable::SequenceEqual(tr, seq21En, comparers::Equals()));
	ASSERT_TRUE(Enumerable::SequenceEqual(tr->Reverse(), seq21Ren, comparers::Equals()));

	ASSERT_TRUE(Enumerable::SequenceEqual(diff1, (DictionaryType::DiffTypePtr)MakeEmptyEnumerable(), comparers::Equals()));
	diff1 = tr->Diff();

	const EntryType seq28[] = {{1, "1"}, {5, "5"}};
	const auto seq28En = EnumerableFromStlIterators(std::begin(seq28), std::end(seq28));
	const auto seq28Ren = EnumerableFromStlIterators(std::rbegin(seq28), std::rend(seq28));

	CHECK_CONTAINS(tr, 3, "33");
	ASSERT_TRUE(tr->TryRemove(3));
	CHECK_NOT_CONTAINS(tr, 3);
	ASSERT_TRUE(Enumerable::SequenceEqual(tr, seq28En, comparers::Equals()));
	ASSERT_TRUE(Enumerable::SequenceEqual(tr->Reverse(), seq28Ren, comparers::Equals()));

	const DiffEntry<EntryType> seq29[] = {{CollectionOp::Removed, {3, "33"}}};
	const auto seq29En = EnumerableFromStlIterators(std::begin(seq29), std::end(seq29));

	ASSERT_TRUE(Enumerable::SequenceEqual(diff1, (DictionaryType::DiffTypePtr)MakeEmptyEnumerable(), comparers::Equals()));
	diff1 = tr->Diff();

	CHECK_NOT_CONTAINS(tr, 3);
	ASSERT_FALSE(tr->TryRemove(3));
	CHECK_NOT_CONTAINS(tr, 3);
	ASSERT_TRUE(Enumerable::SequenceEqual(tr, seq28En, comparers::Equals()));
	ASSERT_TRUE(Enumerable::SequenceEqual(tr->Reverse(), seq28Ren, comparers::Equals()));

	ASSERT_TRUE(Enumerable::SequenceEqual(diff1, seq29En, comparers::Equals()));
	diff1 = tr->Diff();

	CHECK_NOT_CONTAINS(tr, 3);
	tr->Set(3, "3");
	CHECK_CONTAINS(tr, 3, "3");
	ASSERT_TRUE(Enumerable::SequenceEqual(tr, seq3En, comparers::Equals()));
	ASSERT_TRUE(Enumerable::SequenceEqual(tr->Reverse(), seq3Ren, comparers::Equals()));

	const DiffEntry<EntryType> seq30[] = {{CollectionOp::Removed, {3, "33"}}, {CollectionOp::Added, {3, "3"}}};
	const auto seq30En = EnumerableFromStlIterators(std::begin(seq30), std::end(seq30));

	ASSERT_TRUE(Enumerable::SequenceEqual(diff1, seq29En, comparers::Equals()));
	diff1 = tr->Diff();

	tr->Commit();
	tr.reset();

	ASSERT_TRUE(Enumerable::SequenceEqual(diff1, seq30En, comparers::Equals()));
	diff1.reset();

	// 24

	ASSERT_FALSE(s->IsEmpty());
	ASSERT_EQ(s->GetCount(), 3u);
	ASSERT_EQ(Enumerable::Count(s), 3u);

	ASSERT_TRUE(Enumerable::SequenceEqual(s, seq3En, comparers::Equals()));
	ASSERT_TRUE(Enumerable::SequenceEqual(s->Reverse(), seq3Ren, comparers::Equals()));

	ASSERT_TRUE(Enumerable::SequenceEqual(GetValueFromSignal(s->OnChanged()), WrapEnumerable(seq3En, Bind(&MakeDiffEntry<EntryType>, CollectionOp::Added, _1)), comparers::Equals()));

	// 25
}
