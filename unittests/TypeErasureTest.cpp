// Copyright (c) 2011 - 2024, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/TypeErasure.h>

#include <gtest/gtest.h>

using namespace stingray;

namespace
{

	struct WrappedType : public TypeErasureBase
	{
	private:
		bool&	_dtorFlag;
		bool&	_virtualFuncFlag;

	public:
		WrappedType(bool& dtorFlag, bool& virtualFuncFlag) :
			_dtorFlag(dtorFlag),
			_virtualFuncFlag(virtualFuncFlag)
		{ }

		~WrappedType() { _dtorFlag = true; }

		void SetFlag() { _virtualFuncFlag = true; }
	};

	struct VirtualFuncConcept : public function_info<void ()>
	{
		template<typename T>
		static void Apply(T& t)
		{ t.SetFlag(); }
	};

}

TEST(TypeErasureTest, Basics)
{
	bool dtorFlag = false;
	bool virtualFuncFlag = false;

	TypeErasure<TypeList<VirtualFuncConcept> > ptr;
	ptr.Allocate<WrappedType>(wrap_ref(dtorFlag), wrap_ref(virtualFuncFlag));
	ASSERT_TRUE(!dtorFlag);
	ASSERT_TRUE(!virtualFuncFlag);

	ptr.Call<VirtualFuncConcept>();
	ASSERT_TRUE(virtualFuncFlag);

	ptr.Free();
	ASSERT_TRUE(dtorFlag);
}
