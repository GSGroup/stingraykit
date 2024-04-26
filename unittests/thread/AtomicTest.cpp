// Copyright (c) 2011 - 2024, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/function/bind.h>
#include <stingraykit/log/Logger.h>
#include <stingraykit/thread/Thread.h>
#include <stingraykit/thread/atomic/AtomicFlag.h>

#include <stdio.h>

#include <gtest/gtest.h>

using namespace stingray;

namespace
{

	void SpinlockThreadFunc(AtomicFlag::Type& flag)
	{
		static const size_t s_count = 1000;
		for (size_t i = 0; i < s_count; ++i)
		{
			Spinlock l(flag);
			Thread::Sleep(1);
		}
	}


	static const size_t ReadWriteCount = 100000;


	template < typename T >
	void ReaderFunc(const std::vector<T>& allowedValues, const atomic<T>& value)
	{
		for (u32 i = 0; i < ReadWriteCount; ++i)
		{
			T val = value;
			ASSERT_TRUE(std::binary_search(allowedValues.begin(), allowedValues.end(), val))
				<< std::string(StringBuilder() % "Wrong value read: " % val);
		}
	}


	template < typename T >
	void WriterFunc(const std::vector<T>& allowedValues, atomic<T>& value)
	{
		for (u32 i = 0; i < ReadWriteCount; ++i)
		{
			size_t index = i % allowedValues.size();
			value = allowedValues.at(index);
		}
	}


	struct EnumSample
	{
		STINGRAYKIT_ENUM_VALUES(
			V01 = 531394722,
			V02 = 565252517,
			V03 = 855384549,
			V04 = 868886074,
			V05 = 922122641,
			V06 = 1044807237,
			V07 = 1139275291,
			V08 = 1250564738,
			V09 = 1282904262,
			V10 = 1374604046,
			V11 = 1392608377,
			V12 = 1443346686,
			V13 = 1504946537,
			V14 = 1618158229,
			V15 = 1796803282,
			V16 = 1873294573,
			V17 = 1946217173,
			V18 = 1960944470,
			V19 = 2015002471,
			V20 = 2057211470
		);

		STINGRAYKIT_DECLARE_ENUM_CLASS(EnumSample);
	};

}


TEST(AtomicTest, DISABLED_Spinlock)
{
	AtomicFlag::Type flag = 0;
	ThreadPtr thread1 = make_shared_ptr<Thread>("atomic_thread1", Bind(&SpinlockThreadFunc, wrap_ref(flag)));
	ThreadPtr thread2 = make_shared_ptr<Thread>("atomic_thread2", Bind(&SpinlockThreadFunc, wrap_ref(flag)));
	thread1.reset();
	thread2.reset();
	ASSERT_EQ(flag, 0);
}


TEST(AtomicTest, DISABLED_Integral)
{
	u32 values[] =
	{
		2683432021u,
		480524955u,
		1429215490u,
		1807215949u,
		1463311639u,
		1155373792u,
		547603385u,
		2521726457u,
		255409330u,
		3989854004u,
		775138498u,
		3133750636u,
		901017604u,
		4067722585u,
		2064993548u,
		2455969701u,
		3992155551u
	};

	std::vector<u32> allowedValues(values, values + ArraySize(values));
	std::sort(allowedValues.begin(), allowedValues.end());
	atomic<u32> value(allowedValues.front());

	Thread reader("atomicReader", Bind(&ReaderFunc<u32>, wrap_const_ref(allowedValues), wrap_const_ref(value)));
	Thread writer("atomicWriter", Bind(&WriterFunc<u32>, wrap_const_ref(allowedValues), wrap_ref(value)));
}


TEST(AtomicTest, DISABLED_Enum)
{
	std::vector<EnumSample> allowedValues(EnumSample::begin(), EnumSample::end());
	std::sort(allowedValues.begin(), allowedValues.end());
	atomic<EnumSample> value(allowedValues.front());

	Thread reader("atomicReader", Bind(&ReaderFunc<EnumSample>, wrap_const_ref(allowedValues), wrap_const_ref(value)));
	Thread writer("atomicWriter", Bind(&WriterFunc<EnumSample>, wrap_const_ref(allowedValues), wrap_ref(value)));
}
