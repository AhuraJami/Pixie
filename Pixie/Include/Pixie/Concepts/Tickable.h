#ifndef PIXIE_CONCEPTS_TICKABLE_H
#define PIXIE_CONCEPTS_TICKABLE_H

#include <memory>
#include <chrono>

#include "Core/PixieExports.h"
#include "Concepts/Virtual/Tick.h"
#include "Concepts/Virtual/Begin.h"


namespace pixie
{

// TODO(Ahura): Should I prohibit heap allocation of this class?
/**
 * Type erasure class that implements polymorphic Tick concept.
 * Any object that Ticks should be type erased into this class
 */
class Tickable
{
	// Forward declaring Model to silence the compiler warning in Tickable constructor
	template<class T> struct Model;

	// Allow Begin to access private member 'self'
	template<class T> friend void Begin(T&);

	// Allow Tick to access private member 'self'
	template<class T> friend void Tick(T&, std::chrono::nanoseconds);

public:
	/**
	 * (Constructor) Constructs the input object of type T in the heap and stores a unique pointer to it.
	 * @tparam T (Automatically deduced) Type of input object that supposedly implements the concepts of this class
	 * @param [in] x Input object that should implement the concepts
	 */
	template<class T>
	PIXIE_EXPORT Tickable(T x)
			: self(std::make_unique<Model<T>>(std::move(x)))
	{ }

	/** Copy constructor */
	PIXIE_EXPORT Tickable(const Tickable& object)
			: self(object.self->Copy())
	{ }

	/** Default move constructor */
	PIXIE_EXPORT Tickable(Tickable&&) noexcept = default;

	/** Copy assignment operator */
	PIXIE_EXPORT Tickable& operator=(const Tickable& object)
	{ *this = Tickable(object);	return *this; } // breaking into two statements to silence the clang-tidy warning

	/** Move assignment operator */
	PIXIE_EXPORT Tickable& operator=(Tickable&&) noexcept = default;

private:
	/**
	 * Base type erasure interface class that provides Begin and Tick concepts
	 */
	struct Concept : public VirtualTick, public VirtualBegin
	{
		/** Default virtual destructor */
		~Concept() override = default;

		/**
		 * Interface of utility Copy method that is used in Copy constructor of
		 * the parent (i.e. Tickable class)
		 */
		virtual std::unique_ptr<Concept> Copy() const = 0;
	};

	/**
	 * Derived model class that implements the Begin and Tick interfaces
	 * @tparam Type of input object that supposedly implements the concepts
	 */
	template<class T>
	struct Model final : public Concept
	{
		/**
		 * (Constructor) Moves and stores the input object of type T in 'data'
		 * @param [in] x An object that supposedly implements the concepts
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
         * Implementation of virtual Tick method that is called every frame/iteration
         * @param [in] std::chrono::nanoseconds Time it takes to render a single frame/ finish one iteration
         */
		inline void Tick(std::chrono::nanoseconds delta_time) override
		{
			VirtualTick::CallTick(data, delta_time);
		}

		/** Type erased object that supposedly complies with the implemented concepts */
		T data;
	};

private:
	/** A unique pointer to type erased data that implements the inherited concepts */
	std::unique_ptr<Concept> self;
};

} // namespace pixie

#endif //ENGINE_CONCEPTS_TICKABLE_H
