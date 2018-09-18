#ifndef PIXIE_CORE_CORE_H
#define PIXIE_CORE_CORE_H

#include "Pixie/Utility/PixieExports.h"
#include "Pixie/Core/Engine/Engine.h"
#include "Pixie/Core/Scene/Scene.h"
#include "Pixie/Core/Engine/Clock.h"

namespace pixie
{

// TODO(Ahura): Const reference is a safe measure that user doesn't accidentally
// mess up the scene object. However, I think it would be better if these
// accessors were buried deeper so that user don't think of accessing these
// core objects so easily.
// TODO(Ahura): I prefer to have the core and database buried a bit deeper
/**
 * Static Core class that provides direct interface
 */
class PIXIE_API Core
{
public:
	/**
	 * Initializes Pixie's core objects such as Engine and Scene
	 */
	static void Initialize();

	/**
	 * Queries the Engine to start the game loop
	 */
	static void Start();

	/**
	 * Queries the Engine to shut itself down
	 */
	static void Shutdown();

	// TODO(Ahura): Maybe make a copy of all objects at the start and
	// automatically reset them to their original state without
	// requiring the user to implement reset behavior?
	/**
	 * Queries the scene to reset all its registered components
	 * @note Objects must implement reset behavior so that Pixie
	 * can restore them to the state defined by user
	 */
	static void Reset(); // TODO(Ahura): missing implementation

	/**
	 * Destroys the Core.
	 * @note This destroys all the main components (i.e. Engine, Scene, etc.)
	 * and their underlying objects. You can reinitialize pixie by calling
	 * Core::Initialize again.
	 */
	static void Destroy();

	/**
	 * Queries the scene to Create and register a unique instance of the given
	 * game manager
	 * @tparam T (Required) Type of the game manager object that is being created
	 * and registered
	 * @return A pointer to the newly created game manager
	 * @note Do NOT delete the returned pointer
	 */
	template<class T>
	static inline T* CreateGameManager();

	/**
	 * Queries the scene to get the unique instance of the game manager
	 * @tparam T (Required) Type of the registered game manager
	 * @return A pointer to registered game manager
	 * @warning Do NOT delete the returned pointer
	 */
	template<class T>
	static inline T* GetGameManager();

	/**
	 * Queries the scene to Create and add an object of type T into the scene
	 * @tparam T (Required) Type of the object that is being created
	 * @return A pointer to the created object
	 * @warning Do NOT delete the returned pointer
	 * @remark: This level of indirect access is provided to assure that client
	 * will not attempt to access the scene directly
	 */
	template<class T>
	static inline T* CreateObject();

	/**
	 * Queries the scene to Create a component of type T and form its
	 * dependencies
	 * @tparam T (Required) Type of the component that is being created
	 * @return A pointer to the created component
	 * @warning Do NOT delete the returned pointer
	 */
	template<class T>
	static inline T* CreateComponent();

	/**
	 * Queries the scene to copy and register and object that satisfies a pixie's
	 * concept
	 * @tparam T (Automatically Deduced) Type of the object that is being copied
	 * to scene
	 */
	template<class T>
	static inline void CopyObject(T object);

	// TODO(Ahura): Returning by reference is not so ideal.
	// Only Engine needs a reference so it can update it.
	// Maybe make Engine a friend?
	/**
	 * Get a reference to the core clock
	 * @return A reference to the core clock
	 */
	static Clock& GetClock() { return database.clock; }
private:
	/**
	 * Database struct where all core objects are stored
	 */
	struct Database
	{
		/// Engine object
		/// @note Engine is still pretty small so no heap-allocation needed
		/// (for now)
		Engine engine;

		/// Scene where all registered objects live
		/// @note All the objects in the scene are allocated on heap so we can
		/// get away by stack allocation (for now)
		Scene scene;

		/// Specialized Chrono Timer used by engine to measure the rendering time
		Clock clock;
	};

	/// A static reference to database struct where all core objects are stored
	static Database database;

	/// a boolean flag to make sure that Initialize is called before allowing
	/// any of the other method to access the database members
	static bool is_initialized;
};


// =========================================================================
// Template methods definition
// =========================================================================
template<class T>
T* Core::CreateGameManager()
{
	if (is_initialized)
	{
		return Core::database.scene.CreateGameManager<T>();
	}
	return nullptr;
}

template<class T>
T* Core::GetGameManager()
{
	auto& game_manager = Core::database.scene.GetGameManagerRef();
	return game_manager.DynamicCast<T>();
}

template<class T>
T* Core::CreateObject()
{
	if (is_initialized)
	{
		return Core::database.scene.CreateObject<T>();
	}
	return nullptr;
}

template<class T>
T* Core::CreateComponent()
{
	if (is_initialized)
	{
		return Core::database.scene.CreateComponent<T>();
	}
	return nullptr;
}

template<class T>
void Core::CopyObject(T object)
{
	if (is_initialized)
	{
		return Core::database.scene.CopyObject<T>(std::move(object));
	}
}

} //namespace pixie

#endif //PIXIE_CORE_CORE_H
