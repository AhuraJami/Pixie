#include <gtest/gtest.h>
#include <iostream>
#include <future>

#include "Pixie/Core/Core.h"
#include "Pixie/Core/ObjectInitializer.h"
#include "Pixie/Concepts/PObject.h"

using namespace std;
using namespace pixie;

struct C6
{
	bool print_message = true;

	C6() { }

	void Begin()
	{
	}
	void End()
	{
		if(print_message)
			cout << "C6-End\n";
	}
};

struct C5
{
	bool print_message = true;

	C6* c6;

	C5()
	{
		c6 = ObjectInitializer::ConstructComponent<C6>();
	}

	void Begin()
	{
	}

	void Tick()
	{
		c6->print_message = print_message;
	}

	void End()
	{
		if(print_message)
			cout << "C5-End\n";
	}
};

struct C4
{
	bool print_message = true;

	C4() { }

	void Begin()
	{
	}

	void End()
	{
		if(print_message)
			cout << "C4-End\n";
	}
};

struct C3
{
	C4* c4;
	bool print_message = true;

	C3()
	{
		c4 = ObjectInitializer::ConstructComponent<C4>();
	}

	void Begin()
	{
//		cout << "C3-Begin\n";
	}

	void Tick()
	{
		c4->print_message = print_message;
	}

	void End()
	{
		if(print_message)
			cout << "C3-End\n";
	}
};

struct C2
{
	bool print_message = true;

	C2() {}

	void Begin()
	{
//		cout << "C2-Begin\n";
	}

	void End()
	{
		if(print_message)
			cout << "C2-End\n";
	}
};

struct C1
{
	C2* c2;
	C3* c3;
	bool print_message = true;

	C1()
	{
		c2 = ObjectInitializer::ConstructComponent<C2>();
		c3 = ObjectInitializer::ConstructComponent<C3>();
	}

	void Begin()
	{
//		cout << "C1-Begin\n";

		++num;
	}

	void Tick()
	{
		c2->print_message = print_message;
		c3->print_message = print_message;
	}

	void End()
	{
		if(print_message)
			cout << "C1-End\n";
	}

	int num = 0;
};

class Agent1
{
public:
	C1* c1;
	C5* c5;
	bool print_message = true;

	Agent1()
	{
		c1 = ObjectInitializer::ConstructComponent<C1>();
		c5 = ObjectInitializer::ConstructComponent<C5>();
	}

	void Begin()
	{
//		cout << "Agent1-Begin\n";
	}

	void Tick()
	{
		c1->print_message = print_message;
		c5->print_message = print_message;
	}

	void End()
	{
		if(print_message)
			cout << "Agent1-End\n";
	}
};


class Agent2
{
public:
	C3* c3;
	C1* c1;

	bool print_message = true;

	Agent2()
	{
		c3 = ObjectInitializer::ConstructComponent<C3>();
		c1 = ObjectInitializer::ConstructComponent<C1>();
	}

	void Begin()
	{
//		cout << "Agent2-Begin\n";
	}

	void Tick()
	{
		c3->print_message = print_message;
		c1->print_message = print_message;
	}

	void End()
	{
		cout << "Agent2-End\n";
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

	ObjectInitializer::ConstructEntity<Agent1>();
	ObjectInitializer::ConstructEntity<Agent2>();

	::testing::internal::CaptureStdout();

	// Run the code asynchronously
	auto async_engine = StartAsync();

	// Wait for a moment
	async_engine.wait_for(std::chrono::milliseconds{100});

	Core::Shutdown();

	// wait for all the End() to get called
	async_engine.wait_for(std::chrono::milliseconds{10});

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
	 *
	 * Note that objects have priority over tickables as it is expected
	 * of a non-renderable object to handle some logic used by its outer
	 *
	 * Here, C2, C4, C6, are Objects and hence print their message first
	 * in the order they were initialized.
	 */

	EXPECT_STREQ(output.c_str(),
				 "C2-End\n"
				 "C4-End\n"
				 "C6-End\n"
				 "C3-End\n"
				 "C1-End\n"
				 "C5-End\n"
				 "Agent1-End\n"

				 "C4-End\n"
				 "C2-End\n"
				 "C4-End\n"
				 "C3-End\n"
				 "C3-End\n"
				 "C1-End\n"
				 "Agent2-End\n"

	);
}


TEST(SceneForestTest, VerifyReturnPointerRemainsValid)
{
	Core::Initialize();

	auto* agent1 = ObjectInitializer::ConstructEntity<Agent1>();

	EXPECT_EQ(agent1->c1->num, 0);

	agent1->print_message = false;

	// construct a bunch of more objects
	for (int i = 0; i < 1000; ++i)
	{
		auto* temp = ObjectInitializer::ConstructEntity<Agent1>();
		temp->print_message = false;
	}

	auto async_engine = StartAsync();

	// Wait for a moment
	async_engine.wait_for(std::chrono::milliseconds{100});

	Core::Shutdown();

	async_engine.wait_for(std::chrono::milliseconds{10});

	EXPECT_EQ(agent1->c1->num, 1);
}

class C33
{
public:
	void End()
	{
		cout << "C33-End" << endl;
	}
};

class C32
{
public:
	void Begin()
	{
		cout << "C32-Begin" << endl;
	}

	void Tick()
	{
		cout << "123" << endl;
	}

	void End()
	{
		cout << "C32-End" << endl;
	}
};

class C31
{
public:
	int num = 0;

	void Begin()
	{
		num++;
		cout << "C31-Begin" << endl;
	}

	void Tick()
	{
//		cout << "C31-Tick" << endl;
	}
};

class Agent3
{
public:
	PObject c31;
	PObject c32;

	int num = 0;

	Agent3()
	{
		ObjectInitializer::ConstructPObject<C31>(&c31);
		ObjectInitializer::ConstructPObject<C32>(&c32);
	}

	void Begin()
	{
		cout << "Agent3-Begin" << endl;
		c32 = C33();
		num += (c31.StaticCast<C31>()->num);
	}

	void Tick()
	{
	}
};


TEST(SceneForestTest, ChangeComponentDuringRuntime)
{
	Core::Initialize();

	auto agent3 = ObjectInitializer::ConstructEntity<Agent3>();

	::testing::internal::CaptureStdout();

	// Run the code asynchronously
	auto async_engine = StartAsync();

	// Wait for a moment
	async_engine.wait_for(std::chrono::milliseconds{100});

	Core::Shutdown();

	async_engine.wait_for(std::chrono::milliseconds{50});

	std::string output = testing::internal::GetCapturedStdout();

	EXPECT_STREQ(output.c_str(),
				 "C31-Begin\n"
				 "C32-Begin\n"
				 "Agent3-Begin\n"
				 "C33-End\n"
	);

	EXPECT_EQ(agent3->num, 1);
}