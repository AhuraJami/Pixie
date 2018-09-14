#ifndef PIXIE_CORE_SCENE_H
#define PIXIE_CORE_SCENE_H

#include <memory>
#include <vector>
#include <chrono>

#include "Pixie/Core/PixieExports.h"
#include "Pixie/Concepts/Tickable.h"
#include "Pixie/Utility/TypeTraits.h"

namespace pixie
{

// =============================================================================
// Explicit DLL exports
// =============================================================================
// @note Uncomment if used directly by the user. To compile, Tickable will need
// a default constructor.
// template class PIXIE_API std::vector<Tickable, std::allocator<Tickable>>; 


/**
 * Scene class which holds all the objects that are present in the environment
 * @remarks Do not modify registered objects with this class. Treat the class
 * more like a database.
 */
class Scene
{
	
public:
	/** Default constructor */
	Scene() = default;

	/** Default destructor */
	~Scene() = default;

public: // public APIs
	// TODO(Ahura): Is it a good idea to return a pointer
	// and let the user store it in their class?
	// Although convenient, it is problematic if we want to
	// copy the object and all its dependencies.
	// Need a system that determines dependencies such as
	// CreateSubobject<T> and then pass its parent as an argument
	// to register this dependency.
	/**
	 * Creates and adds an object of type T into the scene
	 * @tparam T (Required) Type of the object that is being created and registered
	 * @return A pointer to the created object
	 * @note Do NOT delete the returned pointer
	 */
	template<class T>
	inline T* CreateAndRegisterObject();

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
	/// Registered objects that implement Tick concept
	std::vector<Tickable> tickables;
};

// =============================================================================
// Template methods definition
// =============================================================================
template<class T>
T* Scene::CreateAndRegisterObject()
{
	// TODO(Ahura): Causes the T to be moved at least twice
	// and its destructor to be called twice as well.
	// Probably should restrict the user from managing any
	// resources in the destructor and if they do, they must
	// implement the move constructor as well.
	// Maybe have every class implement a Destroy method that
	// will in turn call it's dependencies Destroy?
	// See above TODO for SubObject registration:
	// Keeping the dependecy chain, we can have user call a
	// single Destroy on parent object, and in turn we destroy
	// all its dependencies.
	Tickable obj = T();
	tickables.emplace_back(std::move(obj));
	return tickables.back().StaticCast<T>();
}


} // namespace pixie

#endif //ENGINE_CORE_SCENE_H
