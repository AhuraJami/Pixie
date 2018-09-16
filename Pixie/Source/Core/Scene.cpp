#include "Pixie/Core/Scene.h"

using namespace pixie;


void Scene::BeginObjects()
{
	// Begin from game manager since it is expected that
	// initial game settings be set here
	Begin(game_manager);

	// Give priority to objects who also implement tick
	for(auto& obj : tickables)
	{
		Begin(obj);
	}

	// Since pixie allows it and it doesn't cost much,
	// call Begin for nontickable objects in hope some of
	// them have implemented it
	for (auto& obj : objects)
	{
		Begin(obj);
	}
}


void Scene::TickObjects()
{
	for(auto& object : tickables)
	{
		Tick(object);
	}

	// Since game manager holds the game logic, we should first let
	// everyone else tick and only then tick the game manager which
	// may then update all the wanted status such as reward, score, etc.
	Tick(game_manager);
}


void Scene::EndObjects()
{
	for(auto& object : tickables)
	{
		End(object);
	}

	// Since pixie allows it and it doesn't cost much,
	// call End for nontickable objects in hope some of
	// them have implemented it
	for (auto& obj : objects)
	{
		End(obj);
	}

	// Just like Tick, let others finish first, then do the final
	// wrap up such storing info, reporting exit status, etc. in
	// game manager
	End(game_manager);
}