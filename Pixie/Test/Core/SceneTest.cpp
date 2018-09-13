#include <gtest/gtest.h>

#include "Pixie/Core/Scene.h"

using namespace pixie;
using namespace std;

class SceneTest : public ::testing::Test
{
protected:
	Scene scene;
};

class SomeObject
{
public:
	void Begin()
	{
		std::cout << "Begin" << std::endl;
		some_number = 10;
	}

	void Tick(std::chrono::nanoseconds delta_time)
	{
		std::cout << "Tick" << std::endl;
	}

	void End()
	{
		std::cout << "End" << std::endl;
	}

	int some_number = 0;
};


TEST_F(SceneTest, RegisterObject)
{
	SomeObject so;
	EXPECT_NO_THROW(scene.Register(so));
}

// TODO(Ahura): Is this a good thing? We have to see...
TEST_F(SceneTest, RegisterAlwaysCopies)
{
	SomeObject so;
	EXPECT_NO_THROW(scene.Register(so));

	::testing::internal::CaptureStdout();
	scene.BeginObjects();
	std::string output = testing::internal::GetCapturedStdout();

	EXPECT_STREQ(output.c_str(), "Begin\n");

	// Verify no change in original object
	EXPECT_EQ(so.some_number, 0);
}

TEST_F(SceneTest, VerifyMethodsGetCalled)
{
	SomeObject so;
	EXPECT_NO_THROW(scene.Register(so));

	::testing::internal::CaptureStdout();
	scene.BeginObjects();
	scene.TickObjects(std::chrono::nanoseconds{1});
	scene.EndObjects();
	std::string output = testing::internal::GetCapturedStdout();

	EXPECT_STREQ(output.c_str(), "Begin\nTick\nEnd\n");
}

