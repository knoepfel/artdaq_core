#ifndef ARTDAQ_CORE_CORE_SHARED_MEMORY_FRAGMENT_MANAGER_HH
#define ARTDAQ_CORE_CORE_SHARED_MEMORY_FRAGMENT_MANAGER_HH

#include "artdaq-core/Core/SharedMemoryManager.hh"
#include "artdaq-core/Data/RawEvent.hh"

namespace artdaq {
	class SharedMemoryFragmentManager : public SharedMemoryManager
	{
	public:
		SharedMemoryFragmentManager(int shm_key, size_t buffer_count, size_t max_buffer_size);
		virtual ~SharedMemoryFragmentManager() =default;

		int WriteFragment(Fragment&& fragment, bool overwrite);
		int ReadFragment(Fragment& fragment);

	};
}

#endif //ARTDAQ_CORE_CORE_SHARED_MEMORY_FRAGMENT_MANAGER_HH