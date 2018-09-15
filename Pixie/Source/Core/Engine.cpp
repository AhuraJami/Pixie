#include <Pixie/Core/Core.h>
#include "Pixie/Core/Engine.h"

#include "Pixie/Core/Scene.h"

using namespace pixie;


void Engine::Start()
{
	// Make sure all main components are set and valid
	if(not scene)
		return;

	// TODO(Ahura): This will keep running if there are
	// no registered objects. Maybe self-kill the processor?
	is_running = true;

	// Call Begin method of all the registered objects (if implemented)
	scene->BeginObjects();

	// Game loop
	while(is_running)
	{
		// start the stop watch
		Core::GetClock().StartTimer();

		// TODO(Ahura): process rendering

		// TODO(Ahura): Process user inputs

		// stop the stop watch
		Core::GetClock().StopTimer();

		// Call Tick member of all the registered objects
		scene->TickObjects();
	}

	// Call End method of all the registered object (if implemented)
	scene->EndObjects();
}

void Engine::Shutdown()
{
	is_running = false;
}
