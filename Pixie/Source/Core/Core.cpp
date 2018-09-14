#include "Pixie/Core/Core.h"

using namespace pixie;

Core::Database Core::database = Core::Database();
bool Core::is_initialized = false;

void Core::Initialize()
{
	database.engine = Engine();
	database.scene = Scene();

	// Give engine a pointer to the scene so that
	// it doesn't constantly query the database
	database.engine.SetScene(&database.scene);

	is_initialized = true;
}

void Core::Start()
{
	if (is_initialized)
	{
		database.engine.Start();
	}
}

void Core::Shutdown()
{
	if (is_initialized)
	{
		database.engine.Shutdown();
	}
}

void Core::Reset()
{
	// TODO(Ahura): Needs implementation
}

void Core::Destroy()
{
	// delete the main components and all the within them

	// Since Core components are stack allocated, we can simply
	// set the is_initialized to false and prevent the old objects
	// to be run again. Hence, forcing the user to call Initialize
	// again to create fresh core componetns, i.e., Engine, scene, etc.

	// But first, make sure the engine is properly Shutdown so that
	// it doesn't access other core components through convenient references
	// stored within it (such as scene).
	Core::Shutdown();

	is_initialized = false;
}
