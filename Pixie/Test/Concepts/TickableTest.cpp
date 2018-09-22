#include <iostream>
#include <gtest/gtest.h>

#include "Pixie/Concepts/Tickable.h"

using namespace pixie;

class TickableObject
{
public:	// @NOTE: The concepts must be public
	void Begin()
	{
		std::cout << "Begin" << std::endl;
	}

	void Tick()
	{
		some_time = some_time + 1;

		std::cout << some_time << "s" << std::endl;
	}

	void End()
	{
		std::cout << "End" << std::endl;
 	}

	int some_time{0};
};

class Derived : TickableObject
{
public:
	void Begin()
	{
		TickableObject::Begin();
		std::cout << "Derived-Begin" << std::endl;
	}

	void Tick()
	{
		TickableObject::Tick();
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


TEST(TickableTest, VerifyValueSemantic)
{
	TickableObject original_object;
	// Object is always copied into Tickable
	Tickable copy_object = original_object;

	::testing::internal::CaptureStdout();
	Tick(copy_object);
	Tick(copy_object);
	Tick(copy_object);
	std::string output = testing::internal::GetCapturedStdout();

	// Verify copy object changes
	EXPECT_STREQ(output.c_str(), "1s\n2s\n3s\n");

	// verify that the original some_time is unchanged
	EXPECT_EQ(original_object.some_time, 0);
}


TEST(TickableTest, BeginDerivedAndParent)
{
	Tickable object = Derived();

	::testing::internal::CaptureStdout();
	Begin(object);
	std::string output = testing::internal::GetCapturedStdout();

	EXPECT_STREQ(output.c_str(), "Begin\nDerived-Begin\n");
}


TEST(TickableTest, TickDerivedAndParent)
{
	Tickable object = Derived();

	::testing::internal::CaptureStdout();
	Tick(object);
	std::string output = testing::internal::GetCapturedStdout();

	EXPECT_STREQ(output.c_str(), "1s\nDerived-Tick\n");
}


TEST(TickableTest, EndDerivedAndParent)
{
	Tickable object = Derived();

	::testing::internal::CaptureStdout();
	End(object);
	std::string output = testing::internal::GetCapturedStdout();

	EXPECT_STREQ(output.c_str(), "End\nDerived-End\n");
}


TEST(TickableTest, OverloadCallBeginForNonTickable)
{
	Tickable obj = NonTickableObject();
	EXPECT_NO_FATAL_FAILURE(Begin(obj));
}

TEST(TickableTest, OverloadCallTickForNonTickable)
{
	Tickable obj = NonTickableObject();

	::testing::internal::CaptureStderr();
	EXPECT_NO_FATAL_FAILURE(Tick(obj));
	std::string output = testing::internal::GetCapturedStderr();

	using T = NonTickableObject;
	std::stringstream buffer;
	buffer << "Error: Object " << /*pixie::type_traits::experimental::type_name<T>()*/ typeid(T).name()
			  << " is specified to comply with Tick concept but does not define a 'Tick' member function.\n"
			  << "If your class already implements Tick then make sure it has a public accessor.\n"
			  << "If not, please define the member function with the following signature within your class.\n"
			  << "Error: " << typeid(T).name() << " Requires\t "
			  << "'void Tick() {}'\n"
			  << "------------------------------------------------------------------------------------------"
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
	void Tick()
	{
		std::cout << "OnlyTick" << std::endl;
	}
};



TEST(TickableTest, OnlyTickObjectTest)
{
	Tickable obj = OnlyTickObject();
	EXPECT_NO_FATAL_FAILURE(Begin(obj));

	::testing::internal::CaptureStdout();
	EXPECT_NO_FATAL_FAILURE(Tick(obj));
	std::string output = testing::internal::GetCapturedStdout();
	EXPECT_STREQ(output.c_str(), "OnlyTick\n");

	EXPECT_NO_FATAL_FAILURE(End(obj));
}



TEST(TickableTest, AccessHiddenObject)
{
	TickableObject object;
	object.some_time = 10;
	Tickable type_erased_object = object; // copied

	// get pointer to copied object
	auto ptr1_object = type_erased_object.StaticCast<TickableObject>();

	// expect values be the same since copy
	EXPECT_EQ(ptr1_object->some_time, object.some_time);

	// modify ptr member to verify copy
	ptr1_object->some_time = 20;

	EXPECT_NE(ptr1_object->some_time, object.some_time);

	// get another ptr
	auto ptr2_object = type_erased_object.DynamicCast<TickableObject>();

	// see the ptrs still point to the same object
	EXPECT_EQ(ptr1_object, ptr2_object);
	EXPECT_EQ(ptr2_object->some_time, 20);

	// verify incorrect cast returns nullptr
	EXPECT_EQ(type_erased_object.DynamicCast<NonTickableObject>(), nullptr);
}