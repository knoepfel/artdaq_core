#ifndef artdaq_core_Core_SharedMemoryManager_hh
#define artdaq_core_Core_SharedMemoryManager_hh

#include <atomic>
#include <string>
#include <deque>

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

		int GetBufferForReading() const;
		int GetBufferForWriting(bool overwrite) const;
		bool ReadyForRead() const;
		size_t ReadReadyCount() const;
		bool ReadyForWrite(bool overwrite) const;
		size_t WriteReadyCount(bool overwrite) const;
		std::deque<int> GetBuffersOwnedByManager();

		void* GetNextWritePos(int buffer);
		size_t BufferDataSize(int buffer);
		void* GetReadPos(int buffer);
		void ResetReadPos(int buffer);
		void IncrementReadPos(int buffer, size_t read);
		bool MoreDataInBuffer(int buffer);

		void SetBufferDestination(int buffer, uint16_t destination_id);
		void MarkBufferFull(int buffer);
		void MarkBufferEmpty(int buffer);
		void ReleaseBuffer(int buffer);

		void GetNewId() { manager_id_ = shm_ptr_->next_id.fetch_add(1); }
		uint16_t GetMaxId() const { return shm_ptr_->next_id.load(); }
		uint16_t GetMyId() const { return manager_id_; }

		bool IsValid() const { return shm_ptr_ ? true : false; }
		size_t size() const { return IsValid() ? shm_ptr_->buffer_count : 0; }

	protected:
		size_t Write(int buffer, void* data, size_t size);
		size_t Read(int buffer, void* data, size_t size);

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
			int buffer_count;
			size_t buffer_size;
			unsigned ready_magic;
		};

		uint8_t* dataStart_() const;
		uint8_t* bufferStart_(int buffer) const;
		ShmBuffer* getBufferInfo_(int buffer) const;


		int shm_segment_id_;
		ShmStruct* shm_ptr_;
		int shm_key_;
		uint16_t manager_id_;
	};

}

#endif // artdaq_core_Core_SharedMemoryManager_hh
