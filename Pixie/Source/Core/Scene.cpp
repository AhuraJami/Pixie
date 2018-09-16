
#include <Pixie/Core/Scene.h>

#include "Pixie/Core/Scene.h"

using namespace pixie;


void Scene::BeginObjects()
{
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
	Tick(game_manager);

	for(auto& object : tickables)
	{
		Tick(object);
	}
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
}

