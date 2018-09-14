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


TEST_F(SceneTest, VerifyMethodsGetCalled)
{
	EXPECT_NO_THROW(scene.CreateAndRegisterObject<SomeObject>());

	::testing::internal::CaptureStdout();
	scene.BeginObjects();
	scene.TickObjects(std::chrono::nanoseconds{1});
	scene.EndObjects();
	std::string output = testing::internal::GetCapturedStdout();

	EXPECT_STREQ(output.c_str(), "Begin\nTick\nEnd\n");
}

