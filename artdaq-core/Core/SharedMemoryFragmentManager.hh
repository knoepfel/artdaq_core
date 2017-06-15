#ifndef ARTDAQ_CORE_CORE_SHARED_MEMORY_FRAGMENT_MANAGER_HH
#define ARTDAQ_CORE_CORE_SHARED_MEMORY_FRAGMENT_MANAGER_HH

#include "artdaq-core/Core/SharedMemoryManager.hh"
#include "artdaq-core/Data/RawEvent.hh"

namespace artdaq {
	/**
	 * \brief The SharedMemoryFragmentManager is a SharedMemoryManager that deals with Fragment transfers using a SharedMemoryManager.
	 */
	class SharedMemoryFragmentManager : public SharedMemoryManager
	{
	public:
		/**
		* \brief SharedMemoryFragmentManager Constructor
		* \param shm_key The key to use when attaching/creating the shared memory segment
		* \param buffer_count The number of buffers in the shared memory
		* \param max_buffer_size The size of each buffer
		* \param stale_buffer_touch_count The maximum number of times a locked buffer can be
		* scanned before being returned to its previous state. This counter is reset upon any operation by the owning SharedMemoryManager.
		*/
		SharedMemoryFragmentManager(int shm_key, size_t buffer_count, size_t max_buffer_size, size_t stale_buffer_touch_count = 0x10000);

		/**
		 * \brief SharedMemoryFragmentManager destructor
		 */
		virtual ~SharedMemoryFragmentManager() =default;

		/**
		 * \brief Write a Fragment to the Shared Memory
		 * \param fragment Fragment to write
		 * \param overwrite Whether to set the overwrite flag
		 * \return 0 on success
		 */
		int WriteFragment(Fragment&& fragment, bool overwrite);

		/**
		 * \brief Read a Fragment from the Shared Memory
		 * \param fragment Output Fragment object
		 * \return 0 on success
		 */
		int ReadFragment(Fragment& fragment);

	};
}

#endif //ARTDAQ_CORE_CORE_SHARED_MEMORY_FRAGMENT_MANAGER_HH