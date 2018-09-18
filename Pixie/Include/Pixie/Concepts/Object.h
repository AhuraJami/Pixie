#ifndef PIXIE_CONCEPTS_OBJECT_H
#define PIXIE_CONCEPTS_OBJECT_H

#include <memory>

#include "Pixie/Utility/PixieExports.h"
#include "Pixie/Concepts/Virtual/Begin.h"
#include "Pixie/Concepts/Virtual/End.h"

namespace pixie
{

// TODO(Ahura): Should I prohibit heap allocation of this class?
/**
 * Type erasure class that implements none of the concepts
 * @NOTE: Any object that doesn't satisfy any of the concepts must be stored in
 * this class
 */
class Object
{
	// Forward declaring Model to silence the compiler warning in Object constructor
	template<class T> struct Model;

	// Since implementation of Begin and End is optional and they are called only once,
	// allow even base objects to optionally implement it.
	template<class T> friend void Begin(T&);
	template<class T> friend void End(T&);

public:
	/**
	 * (Constructor) Constructs the input object of type T in the heap and stores a unique pointer to it.
	 * @tparam T (Automatically deduced) Type of input object that doesn't implement any of
	 * the concepts
	 * @param [in] x Input object that doesn't implement any of the pixie's concepts
	 */
	template<class T>
	PIXIE_EXPORT Object(T x)
			: self(std::make_unique<Model<T>>(std::move(x)))
	{ }

	/** Copy constructor */
	PIXIE_EXPORT Object(const Object& object)
			: self(object.self->Copy())
	{ }

	/** Default move constructor */
	PIXIE_EXPORT Object(Object&&) noexcept = default;

	/** Copy assignment operator */
	PIXIE_EXPORT Object& operator=(const Object& object)
	{ *this = Object(object);	return *this; } // breaking into two statements to silence the clang-tidy warning

	/** Move assignment operator */
	PIXIE_EXPORT Object& operator=(Object&&) noexcept = default;

	/**
	 * Returns a pointer to the type erased object that is stored here
	 * @tparam T (Required) Type of the object that is stored here
	 * @return On successful cast a pointer to the stored object;
	 * otherwise undefined behavior
	 * @warning Do NOT delete this pointer
	 * @remark Use this method only if you are certain that an object of
	 * type T resides in this class; else use DynamicCast<T> and check
	 * for nullptr access before unpacking the returned value
	 */
	template<typename T>
	PIXIE_EXPORT T* StaticCast() const
	{
		return &static_cast<Model<T>*>(self.get())->data;
	}

	/**
	 * Returns a pointer to the type erased object that is stored here
	 * @tparam T (Required) Type of the object that is stored here
	 * @return On successful cast a pointer to the stored object;
	 * otherwise a nullptr
	 * @warning Do NOT delete this pointer
	 */
	template<typename T>
	PIXIE_EXPORT T* DynamicCast() const
	{
		auto ptr = dynamic_cast<Model<T>*>(self.get());
		return ptr ? &ptr->data : nullptr;
	}

private:
	/**
	 * Base type erasure interface class
	 */
	struct Concept : public VirtualBegin, public VirtualEnd
	{
		/** Default virtual destructor */
		virtual ~Concept() = default;

		/**
		 * Interface of utility Copy method that is used in Copy constructor of
		 * the parent (i.e. Object class)
		 */
		virtual std::unique_ptr<Concept> Copy() const = 0;
	};

	/**
	 * Derived model class
	 * @tparam Type of input object that doesn't implement any of the concepts
	 */
	template<class T>
	struct Model final : public Concept
	{
		/**
		 * (Constructor) Moves and stores the input object of type T into 'data'
		 * @param [in] x An object that doesn't implement any of the pixie's concepts
		 */
		Model(T x) : data(std::move(x))	{}

		/**
		 * Implementation of the utility copy interface method that helps with copying
		 * the member 'self' in the parent score
		 */
		inline std::unique_ptr<Concept> Copy() const override
		{
			return std::make_unique<Model>(*this);
		}

		/**
		 * Implementation of virtual Begin method that is called at the beginning of main loop
		 */
		inline void Begin() override
		{
			VirtualBegin::CallBegin(data);
		}

		/**
		 * Implementation of virtual End method that is called at the End of main loop
		 */
		inline void End() override
		{
			VirtualEnd::CallEnd(data);
		}

		/** Type erased object that supposedly complies with the implemented concepts */
		T data;
	};

private:
	/// A unique pointer to type erased data
	std::unique_ptr<Concept> self;
};

} // namespace pixie

#endif //ENGINE_CONCEPTS_TICKABLE_H
