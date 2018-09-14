#ifndef PIXIE_CONCEPTS_VIRTUAL_TICK_H
#define PIXIE_CONCEPTS_VIRTUAL_TICK_H

#include <iostream>
#include <chrono>

#include "Pixie/Core/PixieExports.h"
#include "Pixie/Utility/TypeTraits.h"

namespace pixie
{

/** Utility type trait that checks whether class T implements a 'void Tick method'*/
template<class T, class Duration>
using CheckTick = decltype(std::declval<T>().Tick(std::declval<Duration>()));

/**
 * Template utility type traits boolean that uses detection idiom at compile time to
 * check whether class T implements a Tick method with the following signature:
 * void Tick(std::chrono::nanoseconds);
 * @tparam T Type of the class to check for the presence of Tick method
 */
template<class T>
constexpr bool HasTick = pixie::type_traits::is_detected_v<CheckTick, T, std::chrono::nanoseconds>;

/**
 * Free Tick function that is called from the main game loop which will then redirects
 * the call to the Tick method of input object of type T
 * @tparam T (Automatically deduced) - Type of the concept object that implements the Virtual Tick  concept
 * @param [in] object Concept object that implements the Virtual Tick concept and holds a
 * reference to the actual object who implements 'void Tick(std::chrono::nanoseconds)'
 * @param [in] delta_time Time it takes to render a single frame/ finish one iteration
 */
template<typename T>
PIXIE_EXPORT inline void Tick(T& object, std::chrono::nanoseconds delta_time)
{
	object.self->Tick(delta_time);
}


class VirtualTick
{
public:
    /** Default virtual destructor */
    virtual ~VirtualTick() = default;

	/**
	 * Interface of the virtual Tick function that is called every frame/iteration
	 * @param [in] std::chrono::nanoseconds Time it takes to render a single frame/ finish one iteration
	 * @note All classes that want to comply with this concept must define a public Tick member function
	 */
	virtual void Tick(std::chrono::nanoseconds) = 0;

protected:
    /**
     * Overloaded method that calls the Tick method of the object that implements it
     * @tparam T Automatically deduced - Type of class that implements a Tick method
     * @param [in] data Class that derives from this concept and implements a Tick method
     * @param [in] delta_time Time it takes to render a single frame/ finish one iteration
     */
	template<class T, typename
	std::enable_if_t<HasTick<T> != 0> * = nullptr>
	static inline void CallTick(T& data, std::chrono::nanoseconds delta_time)
	{
		data.Tick(delta_time);
	}


    /**
     * Overloaded method that is invoked when the object of type T does not have a Tick method
     * but still is defined to comply with this concept
     * @tparam T Automatically deduced - Type of class that implements a Tick method
     * @param [in] data Class that derives from this concept and implements a Tick method
     * @param [in] delta_time Time it takes to render a single frame/ finish one iteration
     */
	template<class T, typename
	std::enable_if<HasTick<T> == 0>::type* = nullptr>
	static inline void CallTick(T&, std::chrono::nanoseconds)
	{
		std::cerr << "Error: Object " << /*pixie::type_traits::experimental::type_name<T>()*/ typeid(T).name() << " is specified to comply with "
				  << "Tick concept but does not define a 'Tick' member function.\n"
				  << "If your class already implements Tick then make sure it has a public accessor.\n"
				  << "If not, please define the member function with the following signature within your class.\n"
				  << "Error: " << typeid(T).name()   << " Requires\t "
				  << "'void Tick(std::chrono::nanoseconds delta_time) {}'\n"
				  << "--------------------------------------------------------------------------"
				  << std::endl;
		std::cerr.flush();
	}
};

} //namespace pixie

#endif //ENGINE_CONCEPTS_VIRTUAL_TICK_H
