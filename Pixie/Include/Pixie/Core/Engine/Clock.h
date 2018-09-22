#ifndef PIXIE_CORE_ENGINE_CHRONO_H
#define PIXIE_CORE_ENGINE_CHRONO_H

#include <chrono>

namespace pixie
{

// TODO(Ahura): Requires complete unit tests when a renderer is added
/**
 * Convenient class used exclusively by the Engine to measure and record
 * the rendering time. A utility Time class is also provided to allow user
 * to only read the delta_time variable that is stored here.
 */
class Clock
{
	// Allow Engine to use the simple stopwatch methods of this class
	// which essentially record the rendering time
	friend class Engine;

	using steady_clock = std::chrono::steady_clock;

public:
	/** Default constructor  */
	Clock() = default;

	/** Default public destructor */
	~Clock() = default;

	/**
	 * Returns time it took to render the current frame
	 * @return Time taken to render the current frame in seconds
	 */
	float GetDeltaTimeInSeconds() const
	{
		return delta_time_seconds;
	}

	/**
	 * Returns time it took to render the current frame
	 * @return Time taken to render a single frame in std::chrono::nanoseconds
	 */
	std::chrono::nanoseconds GetDeltaTimeInChrono() const
	{
		return delta_time_chrono;
	}

private:
	/**
	 * Start the stopwatch timer
	 */
	void StartTimer()
	{
		start = steady_clock::now();
	}

	/**
	 * Stop the timer
	 * @note Chrono automatically records the time between calls to StartTimer and StopTimer
	 */
	void StopTimer()
	{
		RecordDeltaTime();
	}

	/**
	 * Record the time between consecutive calls to StartTimer and StopTimer
	 */
	void RecordDeltaTime()
	{
		using namespace std::chrono;
		using float_seconds = duration<float, std::ratio<1>>;

		delta_time_chrono = steady_clock::now() - start;
		delta_time_seconds = duration_cast<float_seconds>(delta_time_chrono).count();
	}

private:
	/// Time it takes to render a single frame (in chrono::nanoseconds)
	std::chrono::nanoseconds delta_time_chrono{0};

	/// Time it takes to render a single frame in seconds
	/// @note Converted from chrono::nanoseconds to float seconds
	/// @remark It is expected that float seconds be used more often than
	/// std::chrono itself. Therefore, we do a single conversion here to
	/// potentially prevent many individual chrono to seconds conversions
	float delta_time_seconds{0.0f};

	/// Time point set by StartTimer at the beginning of a new iteration
	/// of the main game loop
	steady_clock::time_point start;
};

} // namespace pixie

#endif //PIXIE_CORE_ENGINE_CHRONO_H
