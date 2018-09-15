#include "Pixie/Core/Scene.h"

using namespace pixie;


void Scene::BeginObjects()
{
	for(auto& object : tickables)
	{
		Begin(object);
	}
}


void Scene::TickObjects()
{
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
}
