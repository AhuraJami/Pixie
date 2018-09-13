#include "Core/Scene.h"

using namespace pixie;


void Scene::BeginObjects()
{
	for(auto& object : tickables)
	{
		Begin(object);
	}
}


void Scene::TickObjects(std::chrono::nanoseconds delta_time)
{
	for(auto& object : tickables)
	{
		Tick(object, delta_time);
	}
}


void Scene::EndObjects()
{
	for(auto& object : tickables)
	{
		End(object);
	}
}
