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
		SharedMemoryManager(int shm_key, size_t buffer_count, size_t max_buffer_size, size_t stale_buffer_touch_count = 0x10000);

		/**
		 * \brief SharedMemoryManager Destructor
		 */
		virtual ~SharedMemoryManager() noexcept;

		int GetBufferForReading();
		int GetBufferForWriting(bool overwrite);
		bool ReadyForRead() const;
		size_t ReadReadyCount() const;
		bool ReadyForWrite(bool overwrite) const;
		size_t WriteReadyCount(bool overwrite) const;
		std::deque<int> GetBuffersOwnedByManager();

		size_t BufferDataSize(int buffer);
		void ResetReadPos(int buffer);
		void IncrementReadPos(int buffer, size_t read);
		bool MoreDataInBuffer(int buffer);

		bool CheckBuffer(int buffer, BufferSemaphoreFlags flags);

		void MarkBufferFull(int buffer, int destination = -1);
		void MarkBufferEmpty(int buffer);
		void ResetBuffer(int buffer);

		void GetNewId() { if(manager_id_ == 0xFFFF) manager_id_ = shm_ptr_->next_id.fetch_add(1); }
		uint16_t GetMaxId() const { return shm_ptr_->next_id.load() - 1; }
		uint16_t GetMyId() const { return manager_id_; }

		bool IsValid() const { return shm_ptr_ ? true : false; }
		size_t size() const { return IsValid() ? shm_ptr_->buffer_count : 0; }

		size_t Write(int buffer, void* data, size_t size);
		bool Read(int buffer, void* data, size_t size);

	protected:
		void* GetReadPos(int buffer);
		void* GetWritePos(int buffer);

	private:
		struct ShmBuffer
		{
			size_t writePos;
			size_t readPos;
			std::atomic<BufferSemaphoreFlags> sem;
			std::atomic<int16_t> sem_id;
			std::atomic<size_t> buffer_ping_count;
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
		bool checkBuffer_(ShmBuffer* buffer, BufferSemaphoreFlags flags, bool exceptions = true);


		int shm_segment_id_;
		ShmStruct* shm_ptr_;
		int shm_key_;
		uint16_t manager_id_;
		size_t max_ping_count_;
	};

}

#endif // artdaq_core_Core_SharedMemoryManager_hh
