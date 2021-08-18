#ifndef UNITTESTS_MOCKS_DESTRUCTIBLE_H
#define UNITTESTS_MOCKS_DESTRUCTIBLE_H

#include <stingraykit/shared_ptr.h>

#include <gmock/gmock.h>

namespace stingray {
namespace mock
{

	struct Destructible
	{
		virtual ~Destructible() { }

		void Invoke() { }
	};
	STINGRAYKIT_DECLARE_PTR(Destructible);


	struct MockDestructible : public Destructible
	{
		~MockDestructible() { Die(); }

		MOCK_METHOD0(Die, void());
	};
	STINGRAYKIT_DECLARE_PTR(MockDestructible);

}}

#endif
