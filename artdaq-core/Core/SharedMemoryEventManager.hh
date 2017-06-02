#ifndef ARTDAQ_CORE_CORE_SHARED_MEMORY_EVENT_MANAGER_HH
#define ARTDAQ_CORE_CORE_SHARED_MEMORY_EVENT_MANAGER_HH

#include "artdaq-core/Core/SharedMemoryManager.hh"
#include "artdaq-core/Data/RawEvent.hh"

namespace artdaq {
	class SharedMemoryEventManager : public SharedMemoryManager
	{
	public:
		SharedMemoryEventManager();
		virtual ~SharedMemoryEventManager();

		void AddNewEvent(detail::RawEventHeader header);
		void AddFragment(FragmentPtr frag);

		RawEvent_ptr GetRawEvent();

	};
}

#endif //ARTDAQ_CORE_CORE_SHARED_MEMORY_EVENT_MANAGER_HH