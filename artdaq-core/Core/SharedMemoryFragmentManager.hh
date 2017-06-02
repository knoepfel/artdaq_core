#ifndef ARTDAQ_CORE_CORE_SHARED_MEMORY_FRAGMENT_MANAGER_HH
#define ARTDAQ_CORE_CORE_SHARED_MEMORY_FRAGMENT_MANAGER_HH

#include "artdaq-core/Core/SharedMemoryManager.hh"
#include "artdaq-core/Data/RawEvent.hh"

namespace artdaq {
	class SharedMemoryFragmentManager : public SharedMemoryManager
	{
	public:
		SharedMemoryFragmentManager();
		virtual ~SharedMemoryFragmentManager();

		void WriteFragment(FragmentPtr frag);
		FragmentPtr ReadFragment();

	};
}

#endif //ARTDAQ_CORE_CORE_SHARED_MEMORY_FRAGMENT_MANAGER_HH