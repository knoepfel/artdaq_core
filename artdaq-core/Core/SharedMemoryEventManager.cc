#include "artdaq-core/Core/SharedMemoryEventManager.hh"

artdaq::SharedMemoryEventManager::SharedMemoryEventManager(int shm_key, size_t buffer_count, size_t max_buffer_size)
	: SharedMemoryManager(shm_key,buffer_count,max_buffer_size)
{
	
}