#ifndef PIXIE_CORE_SCENE_H
#define PIXIE_CORE_SCENE_H

#include <memory>
#include <vector>
#include <type_traits>

#include "Pixie/Utility/TypeTraits.h"
#include "Pixie/Concepts/Object.h"
#include "Pixie/Concepts/Tickable.h"

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
	/**
	 * Calls the Begin method of all the registered objects (if implemented)
	 */
	void BeginObjects();

	/**
	 * Calls the Tick method of all the registered objects (must be implemented)
     * @param [in] delta_time Time it takes to render a single frame/ finish one iteration
	 */
	void TickObjects();

	/**
	 * Calls the End method of all the registered objects (if implemented)
	 */
	void EndObjects();

	/**
	 * Creates and registers a unique instance of the given game manager
	 * @tparam T (Required) Type of the game manager object that is being created
	 * and registered
	 * @return A pointer to the newly created game manager
	 * @warning Do NOT delete the returned pointer
	 */
	template<class T>
	inline T* CreateAndRegisterGameManager();

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
	 * @warning Do NOT delete the returned pointer
	 */
	template<class T>
	inline T* CreateAndRegisterObject();

	/**
	 * Copies the input object of type T into the scene
	 * @tparam T (Automatically Deduced) Type of the object that is being copied
	 * and registered into the scene
	 */
	template<class T>
	inline void CopyAndRegisterObject(T object);

public:
	/**
	 * Returns a reference to the type erased instance of the game manager
	 * @return a reference to the type erased game manager
	 */
	Tickable& GetGameManagerRef()
	{
		return game_manager;
	}

private:
	/// Unique Game Manager for this instance of scene
	Tickable game_manager = nullptr;

	/// Registered objects that don't comply with any of the concepts
	std::vector<Object> objects;

	/// Registered objects that implement Tick concept
	std::vector<Tickable> tickables{};
};

// =============================================================================
// Template methods definition
// =============================================================================
template<class T>
T* Scene::CreateAndRegisterGameManager()
{
	game_manager = T();
	return game_manager.StaticCast<T>();
}

//GENERATE_HAS_MEMBER(Tick);

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
	using namespace std;

	if (HasTick<T>)
	{
		Tickable obj = T();
		tickables.emplace_back(move(obj));
		return tickables.back().StaticCast<T>();
	}
	else
	{
		Object obj = T();
		objects.emplace_back(move(obj));
		return objects.back().StaticCast<T>();
	}
}

template<class T>
void Scene::CopyAndRegisterObject(T object)
{
	using namespace std;
	if (is_base_of_v<Tickable, T>)
	{
		tickables.emplace_back(move(object));
	}
	else
	{
		objects.emplace_back(move(object));
	}
}

} // namespace pixie

#endif //ENGINE_CORE_SCENE_H
