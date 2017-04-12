#ifndef artdaq_core_Utilities_TimeUtils_h
#define artdaq_core_Utilities_TimeUtils_h

#include <sys/time.h>
#include <string>

namespace artdaq
{
	/**
 * \brief Namespace to hold useful time-converting functions
 */
	namespace TimeUtils
	{
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
