#ifndef PIXIE_CORE_INITIALIZER_H
#define PIXIE_CORE_INITIALIZER_H

#include "Core.h"
#include "Pixie/Misc/PixieExports.h"
#include "Pixie/Concepts/PObject.h"

namespace pixie
{

/**
 * Static core class that initializes all objects to be used within the core of
 * the Pixie
 */
class PIXIE_API ObjectInitializer
{
public:
	/**
	 * Queries the scene to Create and register a unique instance of the given
	 * game manager
	 * @tparam T (Required) Type of the game manager object that is being created
	 * and registered
	 * @return A pointer to the newly created game manager
	 * @note Do NOT delete the returned pointer
	 */
	template<class T>
	static inline T* ConstructGameManager()
	{
		if (Core::is_initialized)
		{
			return Core::database.scene.CreateGameManager<T>();
		}
		return nullptr;
	}

	/**
	 * Queries the scene to get the unique instance of the game manager
	 * @tparam T (Required) Type of the registered game manager
	 * @return A pointer to registered game manager
	 * @warning Do NOT delete the returned pointer
	 */
	template<class T>
	static inline T* GetGameManager()
	{
		auto& game_manager = Core::database.scene.GetGameManagerRef();
		return game_manager.DynamicCast<T>();
	}

	/**
	 * Queries the scene to Create and add an object of type T into the scene
	 * @tparam T (Required) Type of the object that is being created
	 * @return A pointer to the created object
	 * @warning Do NOT delete the returned pointer
	 * @remark: This level of indirect access is provided to assure that client
	 * will not attempt to access the scene directly
	 */
	template<class T>
	static inline T* ConstructEntity()
	{
		if (Core::is_initialized)
		{
			return Core::database.scene.ConstructEntity<T>();
		}
		return nullptr;
	}

	/**
	 * Queries the scene to Create a component of type T and form its
	 * dependencies
	 * @tparam T (Required) Type of the component that is being created
	 * @return A pointer to the created component
	 * @warning Do NOT delete the returned pointer
	 */
	template<class T>
	static T* ConstructComponent()
	{
		return Core::database.scene.ConstructComponent<T>();
	}

	/**
	 * Queries the scene to populate input PObject with the component
	 * of type T and form all its dependencies. This method will
	 * essentially register the PObject to the execution tree of its
	 * outer entity
	 * @tparam T (Required) Type of the component that is being created
	 * @param A pointer to an empty PObject
	 * @warning You will be the owner of this component, hence you
	 * must store it as a member in your class
	 */
	//TODO(Ahura): What if user pass a pointer to a local PObject that
	// would go out of scope at the end of the constructor call?
	// Need a system that is aware of the object lifetime and will remove
	// its respective pointer and all its dependencies from pixie's database as well
	template<class T>
	static void ConstructPObject(PObject* pobject)
	{
		Core::database.scene.ConstructPObject<T>(pobject);
	}
};

} //namespace pixie

#endif //PIXIE_CORE_INITIALIZER_H
