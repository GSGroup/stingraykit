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
