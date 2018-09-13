#include <iostream>
#include <gtest/gtest.h>
#include <chrono>

#include "Pixie/Concepts/Tickable.h"

using namespace pixie;

class TickableObject
{
public:	// @NOTE: The concepts must be public
	void Begin()
	{
		std::cout << "Begin" << std::endl;
	}

	void Tick(std::chrono::nanoseconds delta_time)
	{
		some_time = some_time + delta_time;
		auto duration = std::chrono::duration_cast<std::chrono::duration<float, std::milli>>(some_time).count();

		std::cout << duration << "ms" << std::endl;
	}

	void End()
	{
		std::cout << "End" << std::endl;
 	}

	std::chrono::nanoseconds some_time{0};
};

class Derived : TickableObject
{
public:
	void Begin()
	{
		TickableObject::Begin();
		std::cout << "Derived-Begin" << std::endl;
	}

	void Tick(std::chrono::nanoseconds delta_time)
	{
		TickableObject::Tick(delta_time);
		std::cout << "Derived-Tick" << std::endl;
	}

	void End()
	{
		TickableObject::End();
		std::cout << "Derived-End" << std::endl;
	}

};


class NonTickableObject
{
};


TEST(TickableTest, TypeErasureTest)
{
	std::vector<Tickable> objects;

	objects.emplace_back(TickableObject());
	objects.emplace_back(NonTickableObject());
	objects.emplace_back(5);

	EXPECT_EQ(objects.size(), 3);
};


TEST(TickableTest, VerifyValueSemantic)
{
	TickableObject original_object;
	// Object is always copied into Tickable
	Tickable copy_object = original_object;

	::testing::internal::CaptureStdout();
	Tick(copy_object, std::chrono::nanoseconds{5000000});
	Tick(copy_object, std::chrono::microseconds{5000});
	Tick(copy_object, std::chrono::milliseconds{5});
	std::string output = testing::internal::GetCapturedStdout();

	// Verify copy object changes
	EXPECT_STREQ(output.c_str(), "5ms\n10ms\n15ms\n");

	// verify that the original some_time is unchanged
	EXPECT_EQ(original_object.some_time, std::chrono::nanoseconds{0});
};


TEST(TickableTest, BeginDerivedAndParent)
{
	Tickable object = Derived();

	::testing::internal::CaptureStdout();
	Begin(object);
	std::string output = testing::internal::GetCapturedStdout();

	EXPECT_STREQ(output.c_str(), "Begin\nDerived-Begin\n");
};


TEST(TickableTest, TickDerivedAndParent)
{
	Tickable object = Derived();

	::testing::internal::CaptureStdout();
	Tick(object, std::chrono::milliseconds{1});
	std::string output = testing::internal::GetCapturedStdout();

	EXPECT_STREQ(output.c_str(), "1ms\nDerived-Tick\n");
};


TEST(TickableTest, EndDerivedAndParent)
{
	Tickable object = Derived();

	::testing::internal::CaptureStdout();
	End(object);
	std::string output = testing::internal::GetCapturedStdout();

	EXPECT_STREQ(output.c_str(), "End\nDerived-End\n");
};


TEST(TickableTest, OverloadCallBeginForNonTickable)
{
	Tickable obj = NonTickableObject();
	EXPECT_NO_FATAL_FAILURE(Begin(obj));
}

TEST(TickableTest, OverloadCallTickForNonTickable)
{
	Tickable obj = NonTickableObject();

	::testing::internal::CaptureStderr();
	EXPECT_NO_FATAL_FAILURE(Tick(obj, std::chrono::seconds{1}));
	std::string output = testing::internal::GetCapturedStderr();

	using T = NonTickableObject;
	std::stringstream buffer;
	buffer << "Error: Object " << typeid(T).name() << " is specified to comply with "
		   << "Tick concept but does not define a 'Tick' member function.\n"
		   << "If your class already implements Tick then make sure it has a public accessor.\n"
		   << "If not, please define the member function with the following signature within your class.\n"
		   << "Error: " << typeid(T).name()   << " Requires\t "
		   << "'void Tick(std::chrono::nanoseconds delta_time) {}'\n"
		   << "--------------------------------------------------------------------------"
		   << std::endl;

	EXPECT_STREQ(output.c_str(), buffer.str().c_str());
}

TEST(TickableTest, OverloadCallEndForNonTickable)
{
	Tickable obj = NonTickableObject();
	EXPECT_NO_FATAL_FAILURE(End(obj));
}



class OnlyTickObject
{
public:
	void Tick(std::chrono::nanoseconds delta_time)
	{
		std::cout << "OnlyTick" << std::endl;
	}
};



TEST(TickableTest, OnlyTickObjectTest)
{
	Tickable obj = OnlyTickObject();
	EXPECT_NO_FATAL_FAILURE(Begin(obj));

	::testing::internal::CaptureStdout();
	EXPECT_NO_FATAL_FAILURE(Tick(obj, std::chrono::seconds{1}));
	std::string output = testing::internal::GetCapturedStdout();
	EXPECT_STREQ(output.c_str(), "OnlyTick\n");

	EXPECT_NO_FATAL_FAILURE(End(obj));
}