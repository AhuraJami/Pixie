#ifndef PIXIE_CORE_SCENE_H
#define PIXIE_CORE_SCENE_H

#include <vector>
#include <type_traits>

#include "Pixie/Core/PixieExports.h"
#include "Pixie/Concepts/Tickable.h"
#include "Pixie/Utility/TypeTraits.h"

namespace pixie
{

/**
 * Scene class which holds all the objects that are present in the environment
 * @remarks Do not modify registered objects with this class. Treat the class
 * more like a database.
 */
class PIXIE_API Scene
{
public:
	/**
	 * Registers input object into the scene
	 * @tparam T (Automatically deduced) Type of the object that is begin registered
	 * @param [in] object An object that implements any of the pixie's concepts
	 * @pre: Original object must be passed into this method, otherwise a copy of the
	 * input object may be registered
	 */
	template<class T>
	inline void Register(const T& object);

	/**
	 * Calls the Begin method of all the registered objects (if implemented)
	 */
	void BeginObjects();

	/**
	 * Calls the Tick method of all the registered objects (must be implemented)
     * @param [in] delta_time Time it takes to render a single frame/ finish one iteration
	 */
	void TickObjects(std::chrono::nanoseconds delta_time);

	/**
	 * Calls the End method of all the registered objects (if implemented)
	 */
	void EndObjects();

private:
	/** Registered objects that implement Tick concept */
	std::vector<Tickable> tickables;
};

// =======================================================================================
// Template methods definition
// =======================================================================================
template<class T>
void Scene::Register(const T& object)
{
	tickables.emplace_back(object);
}

} // namespace pixie

#endif //ENGINE_CORE_SCENE_H
