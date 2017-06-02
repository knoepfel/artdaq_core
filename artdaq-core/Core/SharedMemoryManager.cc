#include <sys/shm.h>
#include "cetlib/exception.h"
#include "artdaq-core/Core/SharedMemoryManager.hh"
#include "tracemf.h"



void artdaq::SharedMemoryManager::MarkBufferEmpty(size_t buffer)
{
	auto shmBuf = getBufferInfo_(buffer);
	if (shmBuf->sem != BufferSemaphoreFlags::Reading) throw cet::exception("AccessViolation") << "Shared Memory buffer is not in the correct state to be marked empty!";
	if (shmBuf->sem_id != manager_id_) throw cet::exception("AccessViolation") << "Shared Memory buffer is not owned by this manager instance!";

	shmBuf->readPos = 0;
	shmBuf->writePos = 0;
	shmBuf->sem_id = -1;
	shmBuf->sem = BufferSemaphoreFlags::Empty;
}

void artdaq::SharedMemoryManager::MarkBufferFull(size_t buffer)
{
	auto shmBuf = getBufferInfo_(buffer);
	if (shmBuf->sem != BufferSemaphoreFlags::Writing) throw cet::exception("AccessViolation") << "Shared Memory buffer is not in the correct state to be marked full!";
	if (shmBuf->sem_id != manager_id_) throw cet::exception("AccessViolation") << "Shared Memory buffer is not owned by this manager instance!";

	shmBuf->sem_id = -1;
	shmBuf->sem = BufferSemaphoreFlags::Full;
}

uint8_t* artdaq::SharedMemoryManager::dataStart_() const
{
	return reinterpret_cast<uint8_t*>(shm_ptr_ + 1) + shm_ptr_->buffer_count * sizeof(ShmBuffer);
}

artdaq::SharedMemoryManager::ShmBuffer* artdaq::SharedMemoryManager::getBufferInfo_(size_t buffer) const
{
	if (buffer >= shm_ptr_->buffer_count) throw cet::exception("ArgumentOutOfRange") << "The specified buffer does not exist!";
	return reinterpret_cast<ShmBuffer*>(reinterpret_cast<uint8_t*>(shm_ptr_ + 1) + buffer * sizeof(ShmBuffer));
}

uint8_t* artdaq::SharedMemoryManager::bufferStart_(size_t buffer) const
{
	if (buffer >= shm_ptr_->buffer_count) throw cet::exception("ArgumentOutOfRange") << "The specified buffer does not exist!";
	return dataStart_() + buffer * shm_ptr_->buffer_size;
}


artdaq::SharedMemoryManager::SharedMemoryManager(int shm_key, size_t buffer_count, size_t max_buffer_size)
	: shm_segment_id_(-1)
	, shm_ptr_(NULL)
	, shm_key_(shm_key)
, manager_id_(-1)
{


	size_t shmSize = buffer_count * (max_buffer_size + sizeof(ShmBuffer)) + sizeof(ShmStruct);

	shm_segment_id_ = shmget(shm_key_, shmSize, 0666);

	if (shm_segment_id_ == -1)
	{
		shm_segment_id_ = shmget(shm_key_, shmSize, IPC_CREAT | 0666);
		manager_id_ = 0;
	}

	TLOG_DEBUG("SharedMemoryManager") << "shm_key == " << shm_key_ << ", shm_segment_id == " << shm_segment_id_ << TLOG_ENDL;

	if (shm_segment_id_ > -1)
	{
		TLOG_DEBUG("SharedMemoryManager")
			<< "Created/fetched shared memory segment with ID = " << shm_segment_id_
			<< " and size " << shmSize
			<< " bytes" << TLOG_ENDL;
		shm_ptr_ = (ShmStruct*)shmat(shm_segment_id_, 0, 0);
		TLOG_DEBUG("SharedMemoryManager")
			<< "Attached to shared memory segment at address "
			<< std::hex << shm_ptr_ << std::dec << TLOG_ENDL;
		if (shm_ptr_ && shm_ptr_ != (void *)-1)
		{
			if (manager_id_ == 0)
			{
				shm_ptr_->next_id = 1;
				shm_ptr_->reader_pos = 0;
				shm_ptr_->writer_pos = 0;
				shm_ptr_->buffer_size = max_buffer_size;
				shm_ptr_->buffer_count = buffer_count;

				for (size_t ii = 0; ii < buffer_count; ++ii)
				{
					getBufferInfo_(ii)->writePos = 0;
					getBufferInfo_(ii)->readPos = 0;
					getBufferInfo_(ii)->sem = BufferSemaphoreFlags::Empty;
					getBufferInfo_(ii)->sem_id = -1;
				}
			}
		}
		else
		{
			TLOG_ERROR("SharedMemoryManager") << "Failed to attach to shared memory segment "
				<< shm_segment_id_ << TLOG_ENDL;
		}
	}
	else
	{
		TLOG_ERROR("SharedMemoryManager") << "Failed to connect to shared memory segment"
			<< ", errno = " << errno << ".  Please check "
			<< "if a stale shared memory segment needs to "
			<< "be cleaned up. (ipcs, ipcrm -m <segId>)" << TLOG_ENDL;
	}
}

artdaq::SharedMemoryManager::~SharedMemoryManager()
{
	TRACE(5, "SharedMemoryManager::~SharedMemoryManager called");
	if (shm_ptr_)
	{
		shmdt(shm_ptr_);
		shm_ptr_ = NULL;
	}

	if (manager_id_ == 0 && shm_segment_id_ > -1)
	{
		shmctl(shm_segment_id_, IPC_RMID, NULL);
	}
	TRACE(5, "SharedMemoryManager::~SharedMemoryManager done");
}

// Local Variables:
// mode: c++
// End:
