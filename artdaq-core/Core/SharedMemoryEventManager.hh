#ifndef ARTDAQ_CORE_CORE_SHARED_MEMORY_EVENT_MANAGER_HH
#define ARTDAQ_CORE_CORE_SHARED_MEMORY_EVENT_MANAGER_HH

#include "artdaq-core/Core/SharedMemoryManager.hh"
#include "artdaq-core/Data/RawEvent.hh"
#include <set>

namespace artdaq {
	class SharedMemoryEventManager : public SharedMemoryManager
	{
	public:
		SharedMemoryEventManager(int shm_key, size_t buffer_count, size_t max_buffer_size, size_t fragment_count);
		virtual ~SharedMemoryEventManager() = default;

		void AddFragment(detail::RawFragmentHeader frag, void* dataPtr);
		bool CheckSpace(Fragment::sequence_id_t seqID);

		std::shared_ptr<detail::RawEventHeader> ReadHeader();
		std::set<Fragment::type_t> GetFragmentTypes();
		std::unique_ptr<Fragments> GetFragmentsByType(Fragment::type_t type);
		void ReleaseBuffer();

	private:
		size_t fragments_per_complete_event_;
		int current_read_buffer_;
		std::shared_ptr<detail::RawEventHeader> current_header_;
	};
}

#endif //ARTDAQ_CORE_CORE_SHARED_MEMORY_EVENT_MANAGER_HH