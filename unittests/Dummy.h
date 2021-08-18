#ifndef UNITTESTS_DUMMY_H
#define UNITTESTS_DUMMY_H

#include <stingraykit/log/Logger.h>

#include <gtest/gtest.h>

using namespace stingray;

struct FireRange
{
	volatile int Counter;

	class Dummy
	{
		FireRange	*parent;
		bool		verbose;
		bool		alive;

	public:
		Dummy(FireRange *parent, bool verbose = false) :
			parent(parent),
			verbose(verbose),
			alive(true)
		{
			parent->Add(this);
			if (verbose)
				Logger::Info() << "CTOR\n";
		}

		Dummy(const Dummy &other) :
			parent(other.parent),
			verbose(other.verbose),
			alive(other.alive)
		{
			parent->Add(this);
			EXPECT_TRUE(alive);
			if (verbose)
				Logger::Info() << "COPY\n";
		}

		~Dummy()
		{
			parent->Remove(this);
			EXPECT_TRUE(alive);
			alive = false;
			if (verbose)
				Logger::Info() << "DTOR\n";
		}

		Dummy& operator=(const Dummy &other)
		{
			alive = other.alive;
			EXPECT_TRUE(alive);
			return *this;
		}
	};

	void Add(Dummy *) { ++Counter; }
	void Remove(Dummy *) { --Counter; }

	FireRange() : Counter() { }

	static void HandleException(const std::exception &e) { FAIL() << std::string("exception") + e.what(); }
};

#endif

