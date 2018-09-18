#include <gtest/gtest.h>
#include <iostream>
#include <future>

#include "Pixie/Core/Core.h"

using namespace std;
using namespace pixie;

struct C6
{
	C6() { }

	void Begin()
	{
		cout << "C6-Begin\n";
	}
	void End()
	{
	}
};

struct C5
{
	C5()
	{
		Core::CreateComponent<C6>();
	}

	void Begin()
	{
		cout << "C5-Begin\n";
	}
};

struct C4
{
	C4() { }

	void Begin()
	{
		cout << "C4-Begin\n";
	}
};

struct C3
{
	C3()
	{
		Core::CreateComponent<C4>();
	}

	void Begin()
	{
		cout << "C3-Begin\n";
	}
};

struct C2
{
	C2() {}

	void Begin()
	{
		cout << "C2-Begin\n";
	}
};

struct C1
{
	C1()
	{
		Core::CreateComponent<C2>();
		Core::CreateComponent<C3>();
	}

	void Begin()
	{
		cout << "C1-Begin\n";
	}
};

class Agent1
{
public:
	Agent1()
	{
		Core::CreateComponent<C1>();
		Core::CreateComponent<C5>();
	}

	void Begin()
	{
		cout << "Agent1-Begin\n";
	}

	void Tick()
	{
	}
};


class Agent2
{
public:
	Agent2()
	{
		Core::CreateComponent<C3>();
		Core::CreateComponent<C1>();
	}

	void Begin()
	{
		cout << "Agent2-Begin\n";
	}

	void Tick()
	{
	}
};


auto StartAsync()
{
	return  std::async(std::launch::async, [&]()
			{
				Core::Start();
			});
}


TEST(SceneForestTest, ObjectsExecutionOrder)
{
	Core::Initialize();

	Core::CreateObject<Agent1>();
	Core::CreateObject<Agent2>();

	::testing::internal::CaptureStdout();

	// Run the code asynchronously
	auto async_engine = StartAsync();

	// Wait for a moment
	async_engine.wait_for(std::chrono::milliseconds{500});

	Core::Shutdown();

	std::string output = testing::internal::GetCapturedStdout();

	/**
	 * There are now two objects with the following construction dependency
	 *
	 * Agent1 -> C1 -> C2
	 * 	   |     |
	 * 	   |	 -> C3 -> C4
	 * 	   |
	 * 	   -> C5 -> C6
	 *
	 * Agent2 -> C3 -> C4
	 * 	   |
	 * 		-> C1 -> C2
	 *			|
	 *			-> C3 -> C4
	 *
	 * Agent1: with such dependency, C2 should be the first one to be fully
	 * constructed, followed by C4, C3, C1, C6, C5, and finally Agent1 itself.
	 * Hence the Call to Begin should start with the first object that was
	 * fully initialized and follow from them all the way to Agent1 itself.
	 */

	EXPECT_STREQ(output.c_str(),
				 "C2-Begin\n"
				 "C4-Begin\n"
				 "C3-Begin\n"
				 "C1-Begin\n"
				 "C6-Begin\n"
				 "C5-Begin\n"
				 "Agent1-Begin\n"

				 "C4-Begin\n"
				 "C3-Begin\n"
				 "C2-Begin\n"
				 "C4-Begin\n"
				 "C3-Begin\n"
				 "C1-Begin\n"
				 "Agent2-Begin\n"
	);
}