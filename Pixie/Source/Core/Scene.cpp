#include "Pixie/Core/Scene/Scene.h"

using namespace pixie;


void Scene::BeginObjects()
{
	// Begin from game manager since it is expected that
	// initial game settings be set here
	Begin(game_manager);

	forest.CallBegin();
}


void Scene::TickObjects()
{
	forest.CallTick();

	// Since game manager holds the game logic, we should first let
	// everyone else tick and only then tick the game manager which
	// may then update all the wanted status such as reward, score, etc.
	Tick(game_manager);
}


void Scene::EndObjects()
{
	forest.CallEnd();

	// Just like Tick, let others finish first, then do the final
	// wrap up such storing info, reporting exit status, etc. in
	// game manager
	End(game_manager);
}