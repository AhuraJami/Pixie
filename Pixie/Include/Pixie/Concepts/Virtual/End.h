#ifndef PIXIE_CONCEPTS_VIRTUAL_END_H
#define PIXIE_CONCEPTS_VIRTUAL_END_H

#include "Pixie/Misc/PixieExports.h"
#include "Pixie/Utility/TypeTraits.h"

namespace pixie
{

/** Utility type trait that checks whether class T implements a 'void End()' method */
template<class T>
using CheckEnd = decltype(std::declval<T>().End());

/**
 * Template utility type traits boolean that uses detection idiom at compile time to check whether class T
 * implements a End method with the following signature:
 * void End();
 * @tparam T Type of the class to check for the presence of End method
 */
template<class T>
constexpr bool HasEnd = pixie::type_traits::is_detected_v<CheckEnd, T>;

/**
 * Free End function that is called from the main game loop which will then redirects the call to the
 * End method of input object of type T
 * @tparam T (Automatically deduced) Type of the concept object that implements the Virtual End concept
 * @param [in] object Concept object that implements the Virtual End concept and holds a reference to the
 * actual object who implements the 'void End()' method
 */
template<typename T>
PIXIE_EXPORT inline void End(T& object)
{
	object.self->End();
}


class VirtualEnd
{
public:
    /** Default virtual destructor */
    virtual ~VirtualEnd() = default;

	/**
	 * Interface of the virtual End function that is called once at the start of game loop
	 * @note Since this method is called only once, implementing it is optional for any class
	 * that wants to comply with this concept. However, if implemented, it must be public access
	 * identifier.
	 */
	virtual void End() = 0;

protected:
    /**
     * Overloaded method that calls the End method of the object that implements it
     * @tparam T (Automatically deduced) Type of class that implements a End method
     * @param [in] data Class that derives from this concept and implements a End method
     */
	template<class T, typename
	std::enable_if_t<HasEnd<T> != 0> * = nullptr>
	static inline void CallEnd(T& data)
	{
		data.End();
	}

    /**
     * Overloaded method that is invoked when the object of type T does not have a End method
     * but still is defined to comply with this concept
     * @tparam T (Automatically deduced) Type of class that implements the End method
     * @param [in] data Class that derives from this concept and implements a End method
     */
	template<class T, typename
	std::enable_if_t<HasEnd<T> == 0> * = nullptr>
	static inline void CallEnd(T&)
	{
		// Do nothing.
	}
};

} //namespace pixie

#endif //PIXIE_CONCEPTS_VIRTUAL_END_H