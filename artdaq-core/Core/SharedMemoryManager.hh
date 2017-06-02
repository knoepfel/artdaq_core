#ifndef artdaq_core_Core_SharedMemoryManager_hh
#define artdaq_core_Core_SharedMemoryManager_hh

#include <atomic>
#include <string>

namespace artdaq
{
	/**
	 * \brief
	 */
	class SharedMemoryManager
	{
	public:

		enum class BufferSemaphoreFlags
		{
			Empty,
			Writing,
			Full,
			Reading
		};

		/**
		 * \brief SharedMemoryManager Constructor
		 *
		 */
		SharedMemoryManager(int shm_key, size_t buffer_count, size_t max_buffer_size);

		/**
		 * \brief SharedMemoryManager Destructor
		 */
		virtual ~SharedMemoryManager() noexcept;

		int GetBufferForReading();
		int GetBufferForWriting();
		bool ReadyForRead() { return GetBufferForReading() != -1; }
		bool ReadyForWrite() { return GetBufferForWriting() != -1; }

		void* GetNextWritePos(size_t buffer);
		void* GetReadPos(size_t buffer);

		void SetBufferDestination(size_t buffer, uint16_t destination_id);
		void MarkBufferFull(size_t buffer);
		void MarkBufferEmpty(size_t buffer);

		void GetNewId() { manager_id_ = shm_ptr_->next_id.fetch_add(1); }

	private:
		struct ShmBuffer
		{
			size_t writePos;
			size_t readPos;
			std::atomic<BufferSemaphoreFlags> sem;
			std::atomic<int16_t> sem_id;
		};

		struct ShmStruct
		{
			std::atomic<unsigned int> reader_pos;
			std::atomic<unsigned int> writer_pos;
			std::atomic<uint16_t> next_id;
			size_t buffer_count;
			size_t buffer_size;
		};

		uint8_t* dataStart_() const;

		ShmBuffer* getBufferInfo_(size_t buffer) const;

		uint8_t* bufferStart_(size_t buffer) const;

		int shm_segment_id_;
		ShmStruct* shm_ptr_;
		int shm_key_;
		uint16_t manager_id_;
	};

}

#endif // artdaq_core_Core_SharedMemoryManager_hh
