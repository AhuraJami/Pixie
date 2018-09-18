#include <gtest/gtest.h>
#include <future>

#include "Pixie/Core/Core.h"
#include "Pixie/Utility/Chrono.h"

using namespace pixie;

class HappyGameManager
{
public:
	void Begin() {}

	void Tick()
	{
		status = true;
	}

	void End() {}

	bool status = false;
};


class HappySubobject
{
public:
	void Tick() {};
};


class HappyObject
{
public:
	HappyObject()
	{
		Core::CreateComponent<HappySubobject>();
		std::cout << "ctor" << std::endl;
	}

	~HappyObject()
	{ std::cout << "dtor" << std::endl; }

	HappyObject(const HappyObject& obj)		: counter(obj.counter)
	{ std::cout << "copy" << std::endl; }

	HappyObject(HappyObject&& obj) noexcept : counter(obj.counter)
	{ std::cout << "move" << std::endl; }

	HappyObject& operator=(const HappyObject& obj) noexcept = default;

	HappyObject& operator=(HappyObject&&) noexcept = default;

	void Begin() { }

	void Tick()
	{
		counter++;

		auto delta_seconds = Chrono::DeltaTimeInSeconds();

		// TODO(Ahura): This test is not so useful. Implement better unit tests once renderer is implemented
		EXPECT_GE(delta_seconds, 0.0f);

		if (10 == counter)
			Core::Shutdown();
	}

	int counter = 0;
};


class BaseObject
{
public:
	void Begin()
	{
		++num;
	}

	int num = 0;
};


TEST(CoreTest, GameLoop)
{
	// Initialize Pixie's core
	Core::Initialize();

	// Query the core to create and register the happy game manager
	Core::CreateGameManager<HappyGameManager>();

	// Get an instance of the game manager
	auto game_manager = Core::GetGameManager<HappyGameManager>();

	EXPECT_FALSE(game_manager->status);

	// Create and register the happy object :)
	auto happy_ptr = Core::CreateObject<HappyObject>();
	auto base_obj_ptr = Core::CreateObject<BaseObject>();

	// Run the code asynchronously
	auto async_engine = std::async(
			std::launch::async, [&]() {
				Core::Start(); 	// Happy Object should call the Core::ShutDown()
								// when it is finished. if it doesn't then
								// the test will fail.
			});

	// Timeout test
	EXPECT_TRUE(
			async_engine.wait_for(std::chrono::milliseconds(2000))
			!=
			std::future_status::timeout
	);

	// this will force the Core::Start() to exit and the thread to die
	Core::Shutdown();

	// verify happy_ptr is changed
	EXPECT_EQ(happy_ptr->counter, 10);

	// Game manager should have ticked too
	EXPECT_TRUE(game_manager->status);

	// Verify Begin was called for base object
	EXPECT_EQ(base_obj_ptr->num, 1);

	// unnecessary since core components are stack allocated and do not need to
	// free any other resources - RAII :)
	Core::Destroy();
}

TEST(CoreTest, SubobjectCreation)
{
	Core::Initialize();

	Core::CreateObject<HappyObject>();

	Core::Destroy();
}