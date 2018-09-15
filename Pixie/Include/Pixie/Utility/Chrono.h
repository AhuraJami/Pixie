#ifndef PIXIE_UTILITY_CHRONO_H
#define PIXIE_UTILITY_CHRONO_H

#include <chrono>
#include <Pixie/Core/Core.h>

#include "Pixie/Core/PixieExports.h"


namespace pixie
{

// TODO(Ahura): Implement convenient duration cast methods
/**
 * Provides utility functions to access delta_time and cast between
 * different durations of chrono
 */
class Chrono
{
public:
	/**
	 * Returns the time it took to render the current frame
	 * @return Time taken to render the current frame in seconds
	 */
	PIXIE_EXPORT static float DeltaTimeInSeconds()
	{
		return Core::GetClock().GetDeltaTimeInSeconds();
	}

	/**
	 * Returns time it took to render the current frame
	 * @return Time taken to render a single frame in std::chrono::nanoseconds
	 */
	PIXIE_EXPORT static std::chrono::nanoseconds DeltaTimeInChrono()
	{
		return Core::GetClock().GetDeltaTimeInChrono();
	}
};

} // namespace pixie

#endif //PIXIE_UTILITY_CHRONO_H
