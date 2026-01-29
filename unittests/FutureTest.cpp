// Copyright (c) 2011 - 2025, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/executor/ExecutionDeferrer.h>
#include <stingraykit/executor/ThreadPool.h>
#include <stingraykit/function/bind.h>
#include <stingraykit/future.h>
#include <stingraykit/thread/CancellationToken.h>
#include <stingraykit/thread/TimedCancellationToken.h>

#include <gtest/gtest.h>

using namespace stingray;

namespace
{

	const int SampleValue = 313;

	ExceptionPtr MakeSampleException()
	{ return MakeExceptionPtr(std::exception()); }

	const size_t SampleThreadsNum = 20;
	const TimeDuration SampleDelay = TimeDuration::FromMilliseconds(300);

	STINGRAYKIT_DECLARE_UNIQ_PTR(Dummy);
	STINGRAYKIT_DECLARE_PTR(Dummy);

	class DestructibleOnlyObject
	{
		STINGRAYKIT_NONCOPYABLE(DestructibleOnlyObject);

	public:
		static unique_ptr<DestructibleOnlyObject> Create()
		{ return unique_ptr<DestructibleOnlyObject>(new DestructibleOnlyObject()); }

	private:
		DestructibleOnlyObject() { }
	};

	class MoveOnlyObject
	{
		STINGRAYKIT_NONCOPYABLE(MoveOnlyObject);
		STINGRAYKIT_DEFAULTMOVABLE(MoveOnlyObject);

	public:
		static MoveOnlyObject Create()
		{ return MoveOnlyObject(); }

	private:
		MoveOnlyObject() { };
	};

	class CopyOnlyObject
	{
		STINGRAYKIT_DEFAULTCOPYABLE(CopyOnlyObject);

	public:
		static CopyOnlyObject Create()
		{ return CopyOnlyObject(); }

	private:
		CopyOnlyObject() { }
	};

	template < typename T >
	struct PromiseValueHelper;

	template < >
	struct PromiseValueHelper<DestructibleOnlyObject&>
	{
	private:
		unique_ptr<DestructibleOnlyObject>	_value;

	public:
		PromiseValueHelper() : _value(DestructibleOnlyObject::Create()) { }

		void Set(promise<DestructibleOnlyObject&>& p)
		{ p.set_value(*_value); }
	};

	template < >
	struct PromiseValueHelper<const DestructibleOnlyObject&>
	{
	private:
		const unique_ptr<DestructibleOnlyObject>	_value;

	public:
		PromiseValueHelper() : _value(DestructibleOnlyObject::Create()) { }

		void Set(promise<const DestructibleOnlyObject&>& p)
		{ p.set_value(*_value); }
	};

	template < >
	struct PromiseValueHelper<MoveOnlyObject>
	{
		void Set(promise<MoveOnlyObject>& p)
		{ p.set_value(MoveOnlyObject::Create()); }
	};

	template < >
	struct PromiseValueHelper<CopyOnlyObject>
	{
		void Set(promise<CopyOnlyObject>& p)
		{
			const CopyOnlyObject value = CopyOnlyObject::Create();
			p.set_value(value);
		}
	};

	template < >
	struct PromiseValueHelper<void>
	{
		void Set(promise<void>& p)
		{ p.set_value(); }

		template < typename FutureType >
		bool Check(FutureType& f)
		{
			f.get();
			return true;
		}
	};

	template < >
	struct PromiseValueHelper<DummyUniqPtr>
	{
	private:
		Dummy*					_dummy;

	public:
		void Set(promise<DummyUniqPtr>& p)
		{
			DummyUniqPtr dummy = make_unique_ptr<Dummy>();
			_dummy = dummy.get();

			p.set_value(std::move(dummy));
		}

		bool Check(future<DummyUniqPtr>& f)
		{
			DummyUniqPtr dummy = f.get();
			return _dummy == dummy.get();
		}

		bool Check(shared_future<DummyUniqPtr>& f)
		{
			const DummyUniqPtr& dummy = f.get();
			return _dummy == dummy.get();
		}
	};

	template < >
	struct PromiseValueHelper<const DummyPtr>
	{
	private:
		const DummyPtr			_dummy;

	public:
		PromiseValueHelper()
			:	_dummy(make_shared_ptr<Dummy>())
		{ }

		void Set(promise<const DummyPtr>& p)
		{ p.set_value(_dummy); }

		bool Check(future<const DummyPtr>& f)
		{
			const DummyPtr dummy = f.get();
			return _dummy == dummy;
		}

		bool Check(shared_future<const DummyPtr>& f)
		{
			const DummyPtr& dummy = f.get();
			return _dummy == dummy;
		}
	};

	template < >
	struct PromiseValueHelper<Dummy&>
	{
	private:
		Dummy			_dummy;

	public:
		void Set(promise<Dummy&>& p)
		{ p.set_value(_dummy); }

		template < typename FutureType >
		bool Check(FutureType& f)
		{
			Dummy& dummy = f.get();
			return &_dummy == &dummy;
		}
	};

	template < >
	struct PromiseValueHelper<const Dummy&>
	{
	private:
		const Dummy			_dummy;

	public:
		PromiseValueHelper() : _dummy() { }

		void Set(promise<const Dummy&>& p)
		{ p.set_value(_dummy); }

		template < typename FutureType >
		bool Check(FutureType& f)
		{
			const Dummy& dummy = f.get();
			return &_dummy == &dummy;
		}
	};

	template < typename T >
	void CheckPromiseNoState(promise<T>& p)
	{
		ASSERT_THROW(p.get_future(), InvalidFuturePromiseState);
		ASSERT_THROW(p.set_exception(MakeSampleException()), InvalidFuturePromiseState);
	}

	template < typename FutureType >
	void CheckFutureNoState(const FutureType& f)
	{
		ASSERT_FALSE(f.valid());
		ASSERT_THROW(f.is_ready(), InvalidFuturePromiseState);
		ASSERT_THROW(f.has_exception(), InvalidFuturePromiseState);
		ASSERT_THROW(f.has_value(), InvalidFuturePromiseState);
	}

	template < typename FutureType >
	void CheckFutureStateIsEmpty(const FutureType& f)
	{
		ASSERT_TRUE(f.valid());
		ASSERT_FALSE(f.is_ready());
		ASSERT_FALSE(f.has_exception());
		ASSERT_FALSE(f.has_value());
	}

	template < typename FutureType >
	void CheckFutureStateHasValue(const FutureType& f)
	{
		ASSERT_TRUE(f.valid());
		ASSERT_TRUE(f.is_ready());
		ASSERT_FALSE(f.has_exception());
		ASSERT_TRUE(f.has_value());
	}

	template < typename FutureType >
	void CheckFutureStateHasException(const FutureType& f)
	{
		ASSERT_TRUE(f.valid());
		ASSERT_TRUE(f.is_ready());
		ASSERT_TRUE(f.has_exception());
		ASSERT_FALSE(f.has_value());
	}

	template < typename FutureType >
	class FutureValueChecker : public function_info<void (const ICancellationToken&)>
	{
	private:
		FutureType			_future;
		int					_value;

	public:
		explicit FutureValueChecker(future<int>&& future, int value) : _future(std::move(future)), _value(value) { }

		void operator () (const ICancellationToken& token)
		{
			ASSERT_EQ(_future.wait(), future_status::ready);
			ASSERT_EQ(_future.get(), _value);
		}
	};

	template < typename FutureType >
	class FutureExceptionChecker : public function_info<void (const ICancellationToken&)>
	{
	private:
		FutureType			_future;

	public:
		explicit FutureExceptionChecker(future<int>&& future) : _future(std::move(future)) { }

		void operator () (const ICancellationToken& token)
		{
			ASSERT_EQ(_future.wait(), future_status::ready);
			ASSERT_ANY_THROW(_future.get());
		}
	};

	struct TestSuiteTemplateNameGenerator
	{
		template < typename Type >
		static std::string GetName(int)
		{
			using RawType = typename Decay<Type>::ValueT;

			StringBuilder sb;

			if (IsConst<typename RemoveReference<Type>::ValueT>::Value)
				sb % "const ";

			sb % TypeInfo(typeid(typename Decay<Type>::ValueT));

			if (IsLvalueReference<Type>::Value)
				sb % "&";

			if (IsRvalueReference<Type>::Value)
				sb % "&&";

			return sb;
		}
	};

	using FutureCreationSuiteTypeList = ::testing::Types<DestructibleOnlyObject, DestructibleOnlyObject&, const DestructibleOnlyObject&, void>;

	template < typename >
	class FutureCreationSuite : public ::testing::Test { };
	TYPED_TEST_SUITE(FutureCreationSuite, FutureCreationSuiteTypeList, TestSuiteTemplateNameGenerator);

	using FutureSetSuiteTypeList = ::testing::Types<MoveOnlyObject, CopyOnlyObject, DestructibleOnlyObject&, const DestructibleOnlyObject&, void>;

	template < typename >
	class FutureSetSuite : public ::testing::Test { };
	TYPED_TEST_SUITE(FutureSetSuite, FutureSetSuiteTypeList, TestSuiteTemplateNameGenerator);

	using FutureGetSuiteTypeList = ::testing::Types<DummyUniqPtr, const DummyPtr, Dummy&, const Dummy&, void>;

	template < typename >
	class FutureGetSuite : public ::testing::Test { };
	TYPED_TEST_SUITE(FutureGetSuite, FutureGetSuiteTypeList, TestSuiteTemplateNameGenerator);

}


TYPED_TEST(FutureCreationSuite, PromiseCtors)
{
	using T = TypeParam;

	{
		promise<T> p;
		CheckFutureStateIsEmpty(p.get_future());
	}

	{
		promise<T> p1;
		promise<T> p2(std::move(p1));

		CheckPromiseNoState(p1);
		CheckFutureStateIsEmpty(p2.get_future());
	}
}


TYPED_TEST(FutureCreationSuite, PromiseAssign)
{
	using T = TypeParam;

	promise<T> p1;
	promise<T> p2;

	ASSERT_EQ(&(p1 = std::move(p2)), &p1);

	CheckFutureStateIsEmpty(p1.get_future());
	CheckPromiseNoState(p2);
}


TYPED_TEST(FutureCreationSuite, PromiseSwap)
{
	using T = TypeParam;

	promise<T> p1;
	promise<T> p2;

	p1.get_future();

	p1.swap(p2);

	CheckFutureStateIsEmpty(p1.get_future());
	ASSERT_THROW(p2.get_future(), FutureAlreadyRetrieved);
}


TYPED_TEST(FutureSetSuite, PromiseSetValue)
{
	using T = TypeParam;

	PromiseValueHelper<T> helper;

	{
		promise<T> p;
		promise<T>(std::move(p));
		ASSERT_THROW(helper.Set(p), InvalidFuturePromiseState);
	}

	{
		promise<T> p;
		helper.Set(p);
		ASSERT_THROW(helper.Set(p), PromiseAlreadySatisfied);
		CheckFutureStateHasValue(p.get_future());
	}

	{
		promise<T> p;
		p.set_exception(MakeSampleException());
		ASSERT_THROW(helper.Set(p), PromiseAlreadySatisfied);
		CheckFutureStateHasException(p.get_future());
	}

	{
		promise<T> p;
		helper.Set(p);
		CheckFutureStateHasValue(p.get_future());
	}
}


TYPED_TEST(FutureSetSuite, TestPromiseSetException)
{
	using T = TypeParam;

	PromiseValueHelper<T> helper;

	{
		promise<T> p;
		promise<T>(std::move(p));
		ASSERT_THROW(p.set_exception(MakeSampleException()), InvalidFuturePromiseState);
	}

	{
		promise<T> p;
		helper.Set(p);
		ASSERT_THROW(p.set_exception(MakeSampleException()), PromiseAlreadySatisfied);
		CheckFutureStateHasValue(p.get_future());
	}

	{
		promise<T> p;
		p.set_exception(MakeSampleException());
		ASSERT_THROW(p.set_exception(MakeSampleException()), PromiseAlreadySatisfied);
		CheckFutureStateHasException(p.get_future());
	}

	{
		promise<T> p;
		p.set_exception(MakeSampleException());
		CheckFutureStateHasException(p.get_future());
	}
}


TYPED_TEST(FutureCreationSuite, FutureCtors)
{
	using T = TypeParam;

	{
		future<T> f;
		CheckFutureNoState(f);
	}

	{
		promise<T> p;
		future<T> f1 = p.get_future();

		future<T> f2(std::move(f1));

		CheckFutureNoState(f1);
		CheckFutureStateIsEmpty(f2);
	}
}


TYPED_TEST(FutureCreationSuite, FutureAssign)
{
	using T = TypeParam;

	future<T> f1;

	promise<T> p;
	future<T> f2 = p.get_future();

	ASSERT_EQ(&(f1 = std::move(f2)), &f1);

	CheckFutureStateIsEmpty(f1);
	CheckFutureNoState(f2);
}


TYPED_TEST(FutureGetSuite, FutureGetValue)
{
	using T = TypeParam;

	PromiseValueHelper<T> helper;

	{
		promise<T> p;

		future<T> f = p.get_future();
		CheckFutureStateIsEmpty(f);

		helper.Set(p);
		CheckFutureStateHasValue(f);

		helper.Check(f);
		CheckFutureNoState(f);
	}

	{
		promise<T> p;

		future<T> f = p.get_future();
		CheckFutureStateIsEmpty(f);

		p.set_exception(MakeSampleException());
		CheckFutureStateHasException(f);

		ASSERT_ANY_THROW(f.get());
		CheckFutureNoState(f);
	}

	{
		optional<promise<T>> p(InPlace);

		future<T> f = p->get_future();
		CheckFutureStateIsEmpty(f);

		p.reset();
		CheckFutureStateHasException(f);

		ASSERT_ANY_THROW(f.get());
		CheckFutureNoState(f);
	}
}


TEST(FutureTest, FutureWait)
{
	ExecutionDeferrerWithTimer deferrer("futureWait");

	{
		promise<void> p;
		const future<void> f = p.get_future();

		CancellationToken token;
		deferrer.Defer(Bind(&CancellationToken::Cancel, wrap_ref(token)), SampleDelay);

		ASSERT_EQ(f.wait(token), future_status::cancelled);
		CheckFutureStateIsEmpty(f);
	}

	{
		promise<void> p;
		const future<void> f = p.get_future();

		ASSERT_EQ(f.wait(TimedCancellationToken(SampleDelay)), future_status::timeout);
		CheckFutureStateIsEmpty(f);
	}

	{
		promise<void> p;
		const future<void> f = p.get_future();

		deferrer.Defer(Bind(&promise<void>::set_value, wrap_ref(p)), SampleDelay);

		ASSERT_EQ(f.wait(), future_status::ready);
		CheckFutureStateHasValue(f);
	}

	{
		promise<void> p;
		const future<void> f = p.get_future();

		deferrer.Defer(Bind(&promise<void>::set_exception, wrap_ref(p), Bind(&MakeSampleException)), SampleDelay);

		ASSERT_EQ(f.wait(), future_status::ready);
		CheckFutureStateHasException(f);
	}

	{
		optional<promise<void>> p(InPlace);
		const future<void> f = p->get_future();

		deferrer.Defer(Bind(&optional<promise<void>>::reset, wrap_ref(p)), SampleDelay);

		ASSERT_EQ(f.wait(), future_status::ready);
		CheckFutureStateHasException(f);
	}
}


TYPED_TEST(FutureCreationSuite, SharedFutureCtors)
{
	using T = TypeParam;

	{
		shared_future<T> f;
		CheckFutureNoState(f);
	}

	{
		promise<T> p;
		future<T> f = p.get_future();

		const shared_future<T> sf1 = f.share();
		const shared_future<T> sf2(sf1);

		CheckFutureStateIsEmpty(sf1);
		CheckFutureStateIsEmpty(sf2);

		p.set_exception(MakeSampleException());

		CheckFutureStateHasException(sf1);
		CheckFutureStateHasException(sf2);
	}

	{
		promise<T> p;
		future<T> f = p.get_future();

		shared_future<T> sf1 = f.share();
		const shared_future<T> sf2(std::move(sf1));

		CheckFutureNoState(sf1);
		CheckFutureStateIsEmpty(sf2);

		p.set_exception(MakeSampleException());

		CheckFutureNoState(sf1);
		CheckFutureStateHasException(sf2);
	}

	{
		promise<T> p;
		future<T> f = p.get_future();

		const shared_future<T> sf(std::move(f));

		CheckFutureNoState(f);
		CheckFutureStateIsEmpty(sf);

		p.set_exception(MakeSampleException());

		CheckFutureNoState(f);
		CheckFutureStateHasException(sf);
	}
}


TYPED_TEST(FutureCreationSuite, SharedFutureAssign)
{
	using T = TypeParam;

	{
		promise<T> p;
		future<T> f = p.get_future();

		shared_future<T> sf1 = f.share();
		shared_future<T> sf2;

		ASSERT_EQ(&(sf2 = sf1), &sf2);

		CheckFutureStateIsEmpty(sf1);
		CheckFutureStateIsEmpty(sf2);

		p.set_exception(MakeSampleException());

		CheckFutureStateHasException(sf1);
		CheckFutureStateHasException(sf2);
	}

	{
		promise<T> p;
		future<T> f = p.get_future();

		shared_future<T> sf1 = f.share();
		shared_future<T> sf2;

		ASSERT_EQ(&(sf2 = std::move(sf1)), &sf2);

		CheckFutureNoState(sf1);
		CheckFutureStateIsEmpty(sf2);

		p.set_exception(MakeSampleException());

		CheckFutureNoState(sf1);
		CheckFutureStateHasException(sf2);
	}
}


TYPED_TEST(FutureGetSuite, SharedFutureGetValue)
{
	using T = TypeParam;

	PromiseValueHelper<T> helper;

	{
		promise<T> p;

		shared_future<T> sf1 = p.get_future();
		shared_future<T> sf2(sf1);

		CheckFutureStateIsEmpty(sf1);
		CheckFutureStateIsEmpty(sf2);

		helper.Set(p);

		CheckFutureStateHasValue(sf1);
		CheckFutureStateHasValue(sf2);

		helper.Check(sf1);
		helper.Check(sf2);

		CheckFutureStateHasValue(sf1);
		CheckFutureStateHasValue(sf2);
	}

	{
		promise<T> p;

		shared_future<T> sf1 = p.get_future();
		shared_future<T> sf2(sf1);

		CheckFutureStateIsEmpty(sf1);
		CheckFutureStateIsEmpty(sf2);

		p.set_exception(MakeSampleException());

		CheckFutureStateHasException(sf1);
		CheckFutureStateHasException(sf2);

		ASSERT_ANY_THROW(sf1.get());
		ASSERT_ANY_THROW(sf2.get());

		CheckFutureStateHasException(sf1);
		CheckFutureStateHasException(sf2);
	}

	{
		optional<promise<T>> p(InPlace);

		shared_future<T> sf1 = p->get_future();
		shared_future<T> sf2(sf1);

		CheckFutureStateIsEmpty(sf1);
		CheckFutureStateIsEmpty(sf2);

		p.reset();

		CheckFutureStateHasException(sf1);
		CheckFutureStateHasException(sf2);

		ASSERT_ANY_THROW(sf1.get());
		ASSERT_ANY_THROW(sf2.get());

		CheckFutureStateHasException(sf1);
		CheckFutureStateHasException(sf2);
	}
}


TEST(FutureTest, SharedFutureWait)
{
	ExecutionDeferrerWithTimer deferrer("sharedFutureWait");

	{
		promise<void> p;
		const shared_future<void> sf1 = p.get_future();
		const shared_future<void> sf2 = sf1;

		CancellationToken token;
		deferrer.Defer(Bind(&CancellationToken::Cancel, wrap_ref(token)), SampleDelay);

		ASSERT_EQ(sf1.wait(token), future_status::cancelled);
		CheckFutureStateIsEmpty(sf1);

		token.Reset();
		deferrer.Defer(Bind(&CancellationToken::Cancel, wrap_ref(token)), SampleDelay);

		ASSERT_EQ(sf2.wait(token), future_status::cancelled);
		CheckFutureStateIsEmpty(sf2);
	}

	{
		promise<void> p;
		const shared_future<void> sf1 = p.get_future();
		const shared_future<void> sf2 = sf1;

		ASSERT_EQ(sf1.wait(TimedCancellationToken(SampleDelay)), future_status::timeout);
		CheckFutureStateIsEmpty(sf1);

		ASSERT_EQ(sf2.wait(TimedCancellationToken(SampleDelay)), future_status::timeout);
		CheckFutureStateIsEmpty(sf2);
	}

	{
		promise<void> p;
		const shared_future<void> sf1 = p.get_future();
		const shared_future<void> sf2 = sf1;

		deferrer.Defer(Bind(&promise<void>::set_value, wrap_ref(p)), SampleDelay);

		ASSERT_EQ(sf1.wait(), future_status::ready);
		CheckFutureStateHasValue(sf1);

		ASSERT_EQ(sf2.wait(), future_status::ready);
		CheckFutureStateHasValue(sf2);
	}

	{
		promise<void> p;
		const shared_future<void> sf1 = p.get_future();
		const shared_future<void> sf2 = sf1;

		deferrer.Defer(Bind(&promise<void>::set_exception, wrap_ref(p), Bind(&MakeSampleException)), SampleDelay);

		ASSERT_EQ(sf1.wait(), future_status::ready);
		CheckFutureStateHasException(sf1);

		ASSERT_EQ(sf2.wait(), future_status::ready);
		CheckFutureStateHasException(sf2);
	}

	{
		optional<promise<void>> p(InPlace);
		const shared_future<void> sf1 = p->get_future();
		const shared_future<void> sf2 = sf1;

		deferrer.Defer(Bind(&optional<promise<void>>::reset, wrap_ref(p)), SampleDelay);

		ASSERT_EQ(sf1.wait(), future_status::ready);
		CheckFutureStateHasException(sf1);

		ASSERT_EQ(sf2.wait(), future_status::ready);
		CheckFutureStateHasException(sf2);
	}
}


TEST(FutureTest, DifferentThreadsFutureValue)
{
	promise<int> p;

	Thread checkValueThread("futureCheckValueThread", FutureValueChecker<future<int>>(p.get_future(), SampleValue));

	Thread::Sleep(SampleDelay);

	p.set_value(SampleValue);
}


TEST(FutureTest, DifferentThreadsFutureException)
{
	{
		promise<int> p;

		Thread checkExceptionThread("futureCheckExceptionThread", FutureExceptionChecker<future<int>>(p.get_future()));

		Thread::Sleep(SampleDelay);

		p.set_exception(MakeSampleException());
	}

	{
		optional<promise<int>> p(InPlace);

		Thread checkExceptionThread("futureCheckExceptionThread", FutureExceptionChecker<future<int>>(p->get_future()));

		Thread::Sleep(SampleDelay);

		p.reset();
	}
}


TEST(FutureTest, DifferentThreadsSharedFutureValue)
{
	promise<int> p;

	const FutureValueChecker<shared_future<int>> checker(p.get_future(), SampleValue);

	ThreadPool threadPool("sharedFutureCheckValueThread", SampleThreadsNum);

	for (size_t i = 0; i < SampleThreadsNum; ++i)
		threadPool.Queue(checker);

	Thread::Sleep(SampleDelay);

	p.set_value(SampleValue);
}


TEST(FutureTest, DifferentThreadsSharedFutureException)
{
	{
		promise<int> p;

		const FutureExceptionChecker<shared_future<int>> checker(p.get_future());

		ThreadPool threadPool("sharedFutureCheckExceptionThread", SampleThreadsNum);

		for (size_t i = 0; i < SampleThreadsNum; ++i)
			threadPool.Queue(checker);

		Thread::Sleep(SampleDelay);

		p.set_exception(MakeSampleException());
	}

	{
		optional<promise<int>> p(InPlace);

		const FutureExceptionChecker<shared_future<int>> checker(p->get_future());

		ThreadPool threadPool("sharedFutureCheckExceptionThread", SampleThreadsNum);

		for (size_t i = 0; i < SampleThreadsNum; ++i)
			threadPool.Queue(checker);

		Thread::Sleep(SampleDelay);

		p.reset();
	}
}
