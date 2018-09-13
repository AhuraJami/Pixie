#ifndef PIXIE_CONCEPTS_VIRTUAL_BEGIN_H
#define PIXIE_CONCEPTS_VIRTUAL_BEGIN_H

#include "Core/PixieExports.h"
#include "Utility/TypeTraits.h"

namespace pixie
{

/** Utility type trait that checks whether class T implements a 'void Begin()' method */
template<class T>
using CheckBegin = decltype(std::declval<T>().Begin());

/**
 * Template utility type traits boolean that uses detection idiom at compile time to check whether class T
 * implements a Begin method with the following signature:
 * void Begin();
 * @tparam T Type of the class to check for the presence of Begin method
 */
template<class T>
constexpr bool HasBegin = pixie::type_traits::is_detected_v<CheckBegin, T>;

/**
 * Free Begin function that is called from the main game loop which will then redirects the call to the
 * Begin method of input object of type T
 * @tparam T (Automatically deduced) - Type of the concept object that implements the Virtual Begin concept
 * @param [in] object Concept object that implements the Virtual Begin concept and holds a reference to the
 * actual object who implements the 'void Begin()' method
 */
template<typename T>
PIXIE_EXPORT inline void Begin(T& object)
{
	object.self->Begin();
}

class VirtualBegin
{
public:
    /** Default virtual destructor */
    virtual ~VirtualBegin() = default;

	/**
	 * Interface of the virtual Begin function that is called once at the start of game loop
	 * @note Since this method is called only once, implementing it is optional for any class
	 * that wants to comply with this concept. However, if implemented, it must be public access
	 * identifier.
	 */
	virtual void Begin() = 0;

protected:

    /**
     * Overloaded method that calls the Begin method of the object that implements it
     * @tparam T Automatically deduced - Type of class that implements a Begin method
     * @param [in] data Class that derives from this concept and implements a Begin method
     */
	template<class T, typename
	std::enable_if_t<HasBegin<T> != 0> * = nullptr>
	static inline void CallBegin(T& data)
	{
		data.Begin();
	}


    /**
     * Overloaded method that is invoked when the object of type T does not have a Begin method
     * but still is defined to comply with this concept
     * @tparam T Automatically deduced - Type of class that implements the Begin method
     * @param [in] data Class that derives from this concept and implements a Begin method
     */
	template<class T, typename
	std::enable_if_t<HasBegin<T> == 0> * = nullptr>
	static inline void CallBegin(T& data)
	{
		// Do nothing.
	}
};

} //namespace pixie

#endif //PIXIE_CONCEPTS_VIRTUAL_BEGIN_H