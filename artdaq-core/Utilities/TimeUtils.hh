#ifndef artdaq_core_Utilities_TimeUtils_h
#define artdaq_core_Utilities_TimeUtils_h

#include <sys/time.h>
#include <string>
#include <chrono>

namespace artdaq
{
	/**
 * \brief Namespace to hold useful time-converting functions
 */
	namespace TimeUtils
	{
		/**
		* We shall use artdaq::detail::seconds as our "standard" duration
		* type. Note that this differs from std::chrono::seconds, which has
		* a representation in some integer type of at least 35 bits.
		*
		* daqrate::duration dur(1.0) represents a duration of 1 second.
		* daqrate::duration dur2(0.001) represents a duration of 1
		* millisecond.
		*/
		typedef std::chrono::duration<double> seconds;
		
		/**
	   * \brief Converts a Unix time to its string representation, in UTC
	   * \param inputUnixTime A time_t Unix time variable
	   * \return std::string representation of Unix time, in UTC
	   */
		std::string convertUnixTimeToString(time_t inputUnixTime);

		/**
		* \brief Converts a Unix time to its string representation, in UTC
		* \param inputUnixTime A struct timeval Unix time variable
		* \return std::string representation of Unix time, in UTC
		*/
		std::string convertUnixTimeToString(struct timeval const& inputUnixTime);

		/**
		* \brief Converts a Unix time to its string representation, in UTC
		* \param inputUnixTime A struct timespec Unix time variable
		* \return std::string representation of Unix time, in UTC
		*/
		std::string convertUnixTimeToString(struct timespec const& inputUnixTime);
	}
}

#endif /* artdaq_core_Utilities_TimeUtils_h */

// Local Variables:
// mode: c++
// End:
