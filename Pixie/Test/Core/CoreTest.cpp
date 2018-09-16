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


class HappyObject
{
public:
	HappyObject()
	{ std::cout << "ctor" << std::endl; }

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


TEST(CoreTest, GameLoop)
{
	// Initialize Pixie's core
	Core::Initialize();

	// Query the core to create and register the happy game manager
	Core::RegisterGameManager<HappyGameManager>();

	// Get an instance of the game manager
	auto game_manager = Core::GetGameManager<HappyGameManager>();

	EXPECT_FALSE(game_manager->status);

	// Create and register the happy object :)
	auto ptr_obj = Core::AddObject<HappyObject>();

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

	// verify ptr_obj is changed
	EXPECT_EQ(ptr_obj->counter, 10);

	// Game manager should have ticked too
	EXPECT_TRUE(game_manager->status);

	// unnecessary since core components are stack allocated and do not need to
	// free any other resources - RAII :)
	Core::Destroy();
}