#ifndef PIXIE_CORE_SCENE_SCENE__H
#define PIXIE_CORE_SCENE_SCENE__H

#include <memory>
#include <vector>
#include <type_traits>

#include "Pixie/Concepts/Object.h"
#include "Pixie/Concepts/Tickable.h"
#include "Pixie/Core/Scene/Forest.h"
#include "Pixie/Misc/Placeholders.h"
#include "Pixie/Utility/TypeTraits.h"
#include "Pixie/Concepts/PObject.h"

namespace pixie
{

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
	inline void BeginObjects();

	/**
	 * Calls the Tick method of all the registered objects (must be implemented)
     * @param [in] delta_time Time it takes to render a single frame/ finish one iteration
	 */
	inline void TickObjects();

	/**
	 * Calls the End method of all the registered objects (if implemented)
	 */
	inline void EndObjects();

	/**
	 * Creates and registers a unique instance of the given game manager
	 * @tparam T (Required) Type of the game manager object that is being created
	 * and registered
	 * @return A pointer to the newly created game manager
	 * @warning Do NOT delete the returned pointer
	 */
	template<class T>
	inline T* CreateGameManager()
	{
		game_manager = T();
		return game_manager.StaticCast<T>();
	}

	/**
	 * Returns a reference to the type erased instance of the game manager
	 * @return a reference to the type erased game manager
	 */
	Tickable& GetGameManagerRef()	 	  { return game_manager; }
	const Tickable& GetGameManagerRef() const {	return game_manager; }

	/**
	 * Creates and adds an object of type T into the scene
	 * @tparam T (Required) Type of the object that is being created and registered
	 * @return A pointer to the created object
	 * @warning Do NOT delete the returned pointer
	 */
	template<class T>
	T* ConstructEntity()
	{
		return forest.ConstructEntity<T>();
	}

	/**
	 * Queries the scene forest to constructs a component of type T
	 * which will also queue the component for registration in its outer's
	 * dependency tree
	 * @tparam T (Required) Type of the component that is being created
	 * @return A pointer to the created component
	 * @warning Do NOT delete the returned pointer
	 */
	template<class T>
	inline T* ConstructComponent()
	{
		return forest.ConstructComponent<T>();
	}

	/**
	 * Queries the scene forest to fill the PObject with a component of type T.
	 * Works similar to ConstructComponent.
	 * @tparam T (Required) Type of the component that is being created
	 * @param A pointer to an empty PObject
	 */
	template<class T>
	inline void ConstructPObject(PObject* pobject)
	{
		return forest.ConstructPObject<T>(pobject);
	}

private:
	/// Forest that holds registered objects grouped by their construction
	/// dependency and sorted by their execution id (tick_group)
	Forest forest = Forest();

	/// Unique Game Manager for this instance of scene
	Tickable game_manager = ConceptPlaceHolder();
};

// =============================================================================
// Inline methods definition
// =============================================================================
inline void Scene::BeginObjects()
{
	// Begin from game manager since it is expected that
	// initial game settings be set here
	Begin(game_manager);

	forest.CallBegin();
}


inline void Scene::TickObjects()
{
	forest.CallTick();

	// Since game manager holds the game logic, we should first let
	// everyone else tick and only then tick the game manager which
	// may then update all the wanted status such as reward, score, etc.
	Tick(game_manager);
}


inline void Scene::EndObjects()
{
	forest.CallEnd();

	// Just like Tick, let others finish first, then do the final
	// wrap up such storing info, reporting exit status, etc. in
	// game manager
	End(game_manager);
}

} // namespace pixie

#endif //PIXIE_CORE_SCENE_SCENE__H
