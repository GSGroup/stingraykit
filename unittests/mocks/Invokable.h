#ifndef UNITTESTS_MOCKS_INVOKABLE_H
#define UNITTESTS_MOCKS_INVOKABLE_H

#include <stingraykit/shared_ptr.h>

#include <gmock/gmock.h>

namespace stingray {
namespace mock
{

	struct Invokable
	{
		virtual ~Invokable() { }

		virtual void Invoke() { }
	};

	struct MockInvokable : public Invokable
	{
		MOCK_METHOD0(Invoke, void());
	};

}}

#endif
