#include <stingraykit/thread/DummyCancellationToken.h>

#include <stingraykit/thread/Thread.h>


namespace stingray
{

	void DummyCancellationToken::Sleep(TimeDuration duration) const
	{ Thread::Sleep(duration); }

}
