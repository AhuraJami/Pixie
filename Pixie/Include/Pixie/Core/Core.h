#ifndef PIXIE_CORE_CORE_H
#define PIXIE_CORE_CORE_H

#include "Pixie/Core/PixieExports.h"
#include "Pixie/Core/Engine.h"
#include "Pixie/Core/Scene.h"

namespace pixie
{

/**
 * Static Core class that provides direct interface
 */
class PIXIE_API Core
{
	// Let Engine access the scene in the database
	friend class Engine;

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
	 * Creates and adds an object of type T into the scene
	 * @tparam T (Required) Type of the object that is being created and reigstered
	 */

	/**
	 * Queries the scene to Create and add an object of type T into the scene
	 * @tparam T (Required) Type of the object that is being created
	 * @return A pointer to the created object
	 * @note Do NOT delete the returned pointer
	 * @remark: This level of indirect access is provided to assure that client
	 * will not attempt to access the scene directly
	 */
	template<class T>
	static inline T* CreateObject();

private:
	/**
	 * Database struct where all core objects are stored
	 */
	struct Database
	{
		/// Engine object
		/// @note Engine is still pretty small so no heap-allocation needed (for now)
		Engine engine;

		/// Scene where all registered objects live
		/// @note All the objects in the scene are allocated on heap so we can get away
		/// by stack allocation (fow now)
		Scene scene;
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
T* Core::CreateObject()
{
	if (is_initialized)
		return Core::database.scene.CreateAndRegisterObject<T>();
	else
		return nullptr;
}

} //namespace pixie

#endif //PIXIE_STATICLIBRARY_H