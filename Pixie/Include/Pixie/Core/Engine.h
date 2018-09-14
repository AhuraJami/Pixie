#ifndef PIXIE_CORE_ENGINE_H
#define PIXIE_CORE_ENGINE_H

#include "Pixie/Core/PixieExports.h"
#include "Pixie/Core/Scene.h"

namespace pixie
{

class PIXIE_API Engine final
{
public:
	/** Default constructor  */
	Engine() = default;

	/** Default destructor */
	~Engine() = default;

	/**
	 * Starts the game loop, processes all input commands and class
	 * Begin, Tick, and End methods of registered qualified objects
	 */
	void Start();

	/**
	 * Completely Stops the game loop, therefore the engine will no longer
	 * processes any command
	 * @note It is applications responsibility to call this function
	 * whenever the ending condition of the game/environment is met,
	 * otherwise the engine will keep running and will not know when
	 * to end the game.
	 */
	void Shutdown();

public:
	// TODO(Ahura): Since a scene alone without Engine is of no use in Pixie, maybe
	// this can be a sink function that takes ownership of the scene object?
	// Or even better let Engine initialize it.
	// Open-Question: what if we want to have two separate scenes rendered
	// simultaneously? Should we have two instances of Engine each with its
	// own scene or multiple instances of scene for a single Engine?
	/**
	 * Sets the Engine's scene pointer to the scene object stored in Core database
	 * @param [in] scene A pointer to the scene that is stored in Core database
	 */
	void SetScene(Scene* in_scene) { this->scene = in_scene; }

private:
	/// A pointer to scene object which is originally stored in Core database
	Scene* scene = nullptr;

	/// The current state of the engine
	bool is_running = false;
};

} // namespace pixie

#endif // PIXIE_CORE_ENGINE_H
