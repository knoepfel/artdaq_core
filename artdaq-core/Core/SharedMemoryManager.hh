#ifndef artdaq_core_Core_SharedMemoryManager_hh
#define artdaq_core_Core_SharedMemoryManager_hh

#include <atomic>
#include <string>
#include <deque>
#include "artdaq-core/Utilities/TimeUtils.hh"
#include <mutex>
#include <unordered_map>

namespace artdaq
{
	/**
	 * \brief The SharedMemoryManager creates a Shared Memory area which is divided into a number of fixed-size buffers.
	 * It provides for multiple readers and multiple writers through a dual semaphore system.
	 */
	class SharedMemoryManager
	{
	public:

		/**
		 * \brief The BufferSemaphoreFlags enumeration represents the different possible "states" of a given shared memory buffer
		 */
		enum class BufferSemaphoreFlags
		{
			Empty, ///< The buffer is empty, and waiting for a writer
			Writing, ///< The buffer is currently being written to
			Full, ///< The buffer is full, and waiting for a reader
			Reading ///< The buffer is currently being read from
		};


		/**
		 * \brief Convert a BufferSemaphoreFlags variable to its string represenatation
		 * \param flag BufferSemaphoreFlags variable to convert
		 * \return String representation of flag
		 */
		static std::string FlagToString(BufferSemaphoreFlags flag)
		{
			switch(flag)
			{
			case BufferSemaphoreFlags::Empty:
				return "Empty";
			case BufferSemaphoreFlags::Writing:
				return "Writing";
			case BufferSemaphoreFlags::Full:
				return "Full";
			case BufferSemaphoreFlags::Reading:
				return "Reading";
			}
			return "Unknown";
		}

		/**
		 * \brief SharedMemoryManager Constructor
		 * \param shm_key The key to use when attaching/creating the shared memory segment
		 * \param buffer_count The number of buffers in the shared memory
		 * \param max_buffer_size The size of each buffer
		 * \param buffer_timeout_us The maximum amount of time a buffer can be left untouched by its owner
		 * before being returned to its previous state.
		 */
		SharedMemoryManager(uint32_t shm_key, size_t buffer_count = 0, size_t max_buffer_size = 0, uint64_t buffer_timeout_us = 10 * 1000000);

		/**
		 * \brief SharedMemoryManager Destructor
		 */
		virtual ~SharedMemoryManager() noexcept;


		/**
		 * \brief Reconnect to the shared memory segment
		 */
		void Attach();

		/**
		 * \brief Finds a buffer that is ready to be read, and reserves it for the calling manager.
		 * \return The id number of the buffer. -1 indicates no buffers available for read.
		 */
		int GetBufferForReading();
		/**
		 * \brief Finds a buffer that is ready to be written to, and reserves it for the calling manager.
		 * \param overwrite Whether to consider buffers that are in the Full and Reading state as ready for write (non-reliable mode)
		 * \return The id number of the buffer. -1 indicates no buffers available for write.
		 */
		int GetBufferForWriting(bool overwrite);
		/**
		 * \brief Determine if any buffers are ready for reading
		 * \return Whether any buffers are ready for reading
		 */
		bool ReadyForRead();
		/**
		 * \brief Count the number of buffers that are ready for reading
		 * \return The number of buffers ready for reading
		 */
		size_t ReadReadyCount();
		/**
		 * \brief Determine if any buffers are ready for writing
		 * \param overwrite Whether to consider buffers that are in the Full and Reading state as ready for write (non-reliable mode)
		 * \return Whether any buffers are ready for writing
		 */
		bool ReadyForWrite(bool overwrite);
		/**
		 * \brief Count the number of buffers that are ready for writing
		 * \param overwrite Whether to consider buffers that are in the Full and Reading state as ready for write (non-reliable mode)
		 * \return The number of buffers ready for writing
		 */
		size_t WriteReadyCount(bool overwrite);
		/**
		 * \brief Get the list of all buffers currently owned by this manager instance.
		 * \return A std::deque<int> of buffer IDs currently owned by this manager instance.
		 */
		std::deque<int> GetBuffersOwnedByManager();

		/**
		 * \brief Get the current size of the buffer's data
		 * \param buffer Buffer ID of buffer
		 * \return Current size of data in the buffer
		 */
		size_t BufferDataSize(int buffer);
		/**
		 * \brief Set the read position of the given buffer to the beginning of the buffer
		 * \param buffer Buffer ID of buffer
		 */
		void ResetReadPos(int buffer);
		/**
		 * \brief Increment the read position for a given buffer
		 * \param buffer Buffer ID of buffer
		 * \param read Number of bytes by which to increment read position
		 */
		void IncrementReadPos(int buffer, size_t read);
		/**
		* \brief Increment the write position for a given buffer
		* \param buffer Buffer ID of buffer
		* \param written Number of bytes by which to increment write position
		*/
		void IncrementWritePos(int buffer, size_t written);
		/**
		 * \brief Determine if more data is available to be read, based on the read position and data size
		 * \param buffer Buffer ID of buffer
		 * \return Whether more data is available in the given buffer.
		 */
		bool MoreDataInBuffer(int buffer);

		/**
		 * \brief Check both semaphore conditions (Mode flag and manager ID) for a given buffer
		 * \param buffer Buffer ID of buffer
		 * \param flags Expected Mode flag
		 * \return Whether the buffer passed the check
		 */
		bool CheckBuffer(int buffer, BufferSemaphoreFlags flags);

		/**
		 * \brief Release a buffer from a writer, marking it Full and ready for a reader
		 * \param buffer Buffer ID of buffer
		 * \param destination If desired, a destination manager ID may be specified for a buffer
		 */
		void MarkBufferFull(int buffer, int destination = -1);
		/**
		 * \brief Release a buffer from a reader, marking it Empty and ready to accept more data
		 * \param buffer Buffer ID of buffer
		 */
		void MarkBufferEmpty(int buffer);
		/**
		 * \brief Resets the buffer from Reading to Full or Writing to Empty. This operation will only have an
		 * effect if performed by the owning manager or if the ping counter is above the maximum value.
		 * \param buffer Buffer ID of buffer
		 */
		void ResetBuffer(int buffer);

		/**
		 * \brief Assign a new ID to the current SharedMemoryManager, if one has not yet been assigned
		 */
		void GetNewId() { if(manager_id_ < 0) manager_id_ = shm_ptr_->next_id.fetch_add(1); }
		/**
		 * \brief Get the number of attached SharedMemoryManagers
		 * \return The number of attached SharedMemoryManagers
		 */
		uint16_t GetAttachedCount() const { return shm_ptr_->next_id.load() - 1; }
		/**
		 * \brief Reset the attached manager count to 0
		 */
		void ResetAttachedCount() { if(manager_id_ == 0) shm_ptr_->next_id = 1; }
		/**
		 * \brief Get the ID number of the current SharedMemoryManager
		 * \return The ID number of the current SharedMemoryManager
		 */
		int GetMyId() const { return manager_id_; }

		/**
		 * \brief Get the rank of the owner of the Shared Memory (artdaq assigns rank to each artdaq process for data flow)
		 * \return The rank of the owner of the Shared Memory
		 */
		int GetRank() const { return shm_ptr_->rank; }

		/**
		 * \brief Set the rank stored in the Shared Memory, if the current instance is the owner of the shared memory
		 * \param rank Rank to set
		 */
		void SetRank(int rank) const { if (manager_id_ == 0) shm_ptr_->rank = rank; }

		/**
		 * \brief Is the shared memory pointer valid?
		 * \return Whether the shared memory pointer is valid
		 */
		bool IsValid() const { return shm_ptr_ ? true : false; }
		/**
		 * \brief Get the number of buffers in the shared memory segment
		 * \return The number of buffers in the shared memory segment
		 */
		size_t size() const { return IsValid() ? shm_ptr_->buffer_count : 0; }

		/**
		 * \brief Write size bytes of data from the given pointer to a buffer
		 * \param buffer Buffer ID of buffer
		 * \param data Source pointer for write
		 * \param size Size of write, in bytes
		 * \return Amount of data written, in bytes
		 */
		size_t Write(int buffer, void* data, size_t size);
		/**
		 * \brief Read size bytes of data from buffer into the given pointer
		 * \param buffer Buffer ID of buffer
		 * \param data Destination pointer for read
		 * \param size Size of read, in bytes
		 * \return Whether the read was successful
		 */
		bool Read(int buffer, void* data, size_t size);

		/**
		 *\brief Write information about the SharedMemory to a string
		 *\return String describing current state of SharedMemory and buffers
		 */
		virtual std::string toString();


		/**
		 * \brief Get the key of the shared memory attached to this SharedMemoryManager
		 * \return The shared memory key
		 */
		uint32_t GetKey() const { return shm_key_; }

	protected:
		/**
		 * \brief Get a pointer to the current read position of the buffer
		 * \param buffer Buffer ID of buffer
		 * \return void* pointer to the buffer's current read position
		 */
		void* GetReadPos(int buffer);
		/**
		 * \brief Get a pointer to the current write position of the buffer
		 * \param buffer Buffer ID of buffer
		 * \return void* pointer to buffer's current write position
		 */
		void* GetWritePos(int buffer);

		/**
		 * \brief Get a pointer to the start position of the buffer
		 * \param buffer Buffer ID of buffer
		 * \return void* pointer to buffer start position
		 */
		void* GetBufferStart(int buffer);

		/**
		 * \brief Detach from the Shared Memory segment, optionally throwing a cet::exception with the specified properties
		 * \param throwException Whether to throw an exception after detaching
		 * \param category Category for the cet::exception
		 * \param message Message for the cet::exception
		 */
		void Detach(bool throwException = false, std::string category = "", std::string message = "");
	private:
		struct ShmBuffer
		{
			size_t writePos;
			size_t readPos;
			std::atomic<BufferSemaphoreFlags> sem;
			std::atomic<int16_t> sem_id;
			std::atomic<uint64_t> buffer_touch_time;
		};

		struct ShmStruct
		{
			std::atomic<unsigned int> reader_pos;
			std::atomic<unsigned int> writer_pos;
			std::atomic<int> next_id;
			int buffer_count;
			size_t buffer_size;
			int rank;
			unsigned ready_magic;
		};

		uint8_t* dataStart_() const;
		uint8_t* bufferStart_(int buffer);
		ShmBuffer* getBufferInfo_(int buffer);
		bool checkBuffer_(ShmBuffer* buffer, BufferSemaphoreFlags flags, bool exceptions = true);
		void touchBuffer_(ShmBuffer* buffer);


		int shm_segment_id_;
		ShmStruct* shm_ptr_;
		uint32_t shm_key_;
		size_t shm_buffer_size_;
		int shm_buffer_count_;
		int manager_id_;
		uint64_t buffer_timeout_us_;
		mutable std::unordered_map<int, std::mutex> buffer_mutexes_;
		mutable std::mutex search_mutex_;
	};

}

#endif // artdaq_core_Core_SharedMemoryManager_hh
