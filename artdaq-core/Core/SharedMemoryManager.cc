#define TRACE_NAME "SharedMemoryManager"
#include <cstring>
#include <vector>
#include <sys/shm.h>
#include "tracemf.h"
#include <signal.h>
#include "cetlib_except/exception.h"
#include "artdaq-core/Core/SharedMemoryManager.hh"
#include "artdaq-core/Utilities/TraceLock.hh"

#define TLVL_DETACH 11

static std::vector<artdaq::SharedMemoryManager*> instances = std::vector<artdaq::SharedMemoryManager*>();

static std::unordered_map<int, struct sigaction> old_actions = std::unordered_map<int, struct sigaction>();
static bool sighandler_init = false;
static void signal_handler(int signum)
{
	// Messagefacility may already be gone at this point, TRACE ONLY!
	TRACE_STREAMER(TLVL_ERROR, &("SharedMemoryManager")[0], 0, 0, 0) << "A signal of type " << signum << " (" << std::string(strsignal(signum)) << ") was caught by SharedMemoryManager. Detaching all Shared Memory segments, then proceeding with default handlers!";
	for (auto ii : instances)
	{
		if(ii) ii->Detach(false, "", "", true);
		ii = nullptr;
	}

	sigset_t set;
	pthread_sigmask(SIG_UNBLOCK, NULL, &set);
	pthread_sigmask(SIG_UNBLOCK, &set, NULL);

	TRACE_STREAMER(TLVL_ERROR, &("SharedMemoryManager")[0], 0, 0, 0) << "Calling default signal handler";
	if (signum != SIGUSR2) {
		sigaction(signum, &old_actions[signum], NULL);
		kill(getpid(), signum); // Only send signal to self
	}
	else {
		// Send Interrupt signal if parsing SIGUSR2 (i.e. user-defined exception that should tear down ARTDAQ)
		sigaction(SIGINT, &old_actions[SIGINT], NULL);
		kill(getpid(), SIGINT); // Only send signal to self
	}
}

artdaq::SharedMemoryManager::SharedMemoryManager(uint32_t shm_key, size_t buffer_count, size_t buffer_size, uint64_t buffer_timeout_us, bool destructive_read_mode)
	: shm_segment_id_(-1)
	, shm_ptr_(NULL)
	, shm_key_(shm_key)
	, manager_id_(-1)
	, buffer_mutexes_()
	, last_seen_id_(0)
{
	requested_shm_parameters_.buffer_count = buffer_count;
	requested_shm_parameters_.buffer_size = buffer_size;
	requested_shm_parameters_.buffer_timeout_us = buffer_timeout_us;
	requested_shm_parameters_.destructive_read_mode = destructive_read_mode;

	instances.push_back(this);
	Attach();

	static std::mutex sighandler_mutex;
	std::unique_lock<std::mutex> lk(sighandler_mutex);

	if (!sighandler_init )//&& manager_id_ == 0) // ELF 3/22/18: Taking out manager_id_==0 requirement as I think kill(getpid()) is enough protection
	{
		sighandler_init = true;
		std::vector<int> signals = { SIGINT, SIGQUIT, SIGILL, SIGABRT, SIGFPE, SIGSEGV, SIGPIPE, SIGALRM, SIGTERM, SIGUSR2 };
		for (auto signal : signals)
		{
			struct sigaction old_action;
			sigaction(signal, NULL, &old_action);

			//If the old handler wasn't SIG_IGN (it's a handler that just
			// "ignore" the signal)
			if (old_action.sa_handler != SIG_IGN)
			{
				struct sigaction action;
				action.sa_handler = signal_handler;
				sigemptyset(&action.sa_mask);
				for (auto sigblk : signals)
				{
					sigaddset(&action.sa_mask, sigblk);
				}
				action.sa_flags = 0;

				//Replace the signal handler of SIGINT with the one described by new_action
				sigaction(signal, &action, NULL);
				old_actions[signal] = old_action;
			}
		}
	}
}


artdaq::SharedMemoryManager::~SharedMemoryManager() noexcept
{
	TLOG(TLVL_DEBUG) << "~SharedMemoryManager called" << TLOG_ENDL;
	Detach();
	TLOG(TLVL_DEBUG) << "~SharedMemoryManager done" << TLOG_ENDL;
}

void artdaq::SharedMemoryManager::Attach()
{
	if (IsValid())
	{
		if (manager_id_ == 0) return;
		Detach();
	}
	auto start_time = std::chrono::steady_clock::now();
	last_seen_id_ = 0;
	size_t shmSize = requested_shm_parameters_.buffer_count * (requested_shm_parameters_.buffer_size + sizeof(ShmBuffer)) + sizeof(ShmStruct);

	shm_segment_id_ = shmget(shm_key_, shmSize, 0666);
	if (shm_segment_id_ == -1 && requested_shm_parameters_.buffer_count > 0 && manager_id_ <= 0)
	{
		TLOG(TLVL_DEBUG) << "Creating shared memory segment with key 0x" << std::hex << shm_key_ << TLOG_ENDL;
		shm_segment_id_ = shmget(shm_key_, shmSize, IPC_CREAT | 0666);
		manager_id_ = 0;
	}

	while (shm_segment_id_ == -1 && TimeUtils::GetElapsedTimeMilliseconds(start_time) < 1000)
	{
		shm_segment_id_ = shmget(shm_key_, shmSize, 0666);

	}
	TLOG(TLVL_DEBUG) << "shm_key == 0x" << std::hex << shm_key_ << ", shm_segment_id == " << shm_segment_id_ << TLOG_ENDL;

	if (shm_segment_id_ > -1)
	{
		TLOG(TLVL_DEBUG)
			<< "Attached to shared memory segment with ID = " << shm_segment_id_
			<< " and size " << shmSize
			<< " bytes" << TLOG_ENDL;
		shm_ptr_ = (ShmStruct*)shmat(shm_segment_id_, 0, 0);
		TLOG(TLVL_DEBUG)
			<< "Attached to shared memory segment at address "
			<< std::hex << (void*)shm_ptr_ << std::dec << TLOG_ENDL;
		if (shm_ptr_ && shm_ptr_ != (void *)-1)
		{
			if (manager_id_ == 0)
			{
				if (shm_ptr_->ready_magic == 0xCAFE1111)
				{
					TLOG(TLVL_ERROR) << "Owner encountered already-initialized Shared Memory!" << TLOG_ENDL;
					exit(-2);
				}
				TLOG(TLVL_DEBUG) << "Owner initializing Shared Memory" << TLOG_ENDL;
				shm_ptr_->next_id = 1;
				shm_ptr_->next_sequence_id = 0;
				shm_ptr_->reader_pos = 0;
				shm_ptr_->writer_pos = 0;
				shm_ptr_->buffer_size = requested_shm_parameters_.buffer_size;
				shm_ptr_->buffer_count = requested_shm_parameters_.buffer_count;
				shm_ptr_->buffer_timeout_us = requested_shm_parameters_.buffer_timeout_us;
				shm_ptr_->destructive_read_mode = requested_shm_parameters_.destructive_read_mode;

				for (int ii = 0; ii < static_cast<int>(requested_shm_parameters_.buffer_count); ++ii)
				{
					getBufferInfo_(ii)->writePos = 0;
					getBufferInfo_(ii)->readPos = 0;
					getBufferInfo_(ii)->sem = BufferSemaphoreFlags::Empty;
					getBufferInfo_(ii)->sem_id = -1;
					getBufferInfo_(ii)->last_touch_time = TimeUtils::gettimeofday_us();
				}

				shm_ptr_->ready_magic = 0xCAFE1111;
			}
			else
			{
				TLOG(TLVL_DEBUG) << "Waiting for owner to initalize Shared Memory" << TLOG_ENDL;
				while (shm_ptr_->ready_magic != 0xCAFE1111) { usleep(1000); }
				TLOG(TLVL_DEBUG) << "Getting ID from Shared Memory" << TLOG_ENDL;
				GetNewId();
				shm_ptr_->lowest_seq_id_read = 0;
				TLOG(TLVL_DEBUG) << "Getting Shared Memory Size parameters" << TLOG_ENDL;
			}
			//last_seen_id_ = shm_ptr_->next_sequence_id;
			TLOG(TLVL_DEBUG) << "Initialization Complete: "
				<< "key: 0x" << std::hex << shm_key_
				<< ", manager ID: " << manager_id_
				<< ", Buffer size: " << std::to_string(shm_ptr_->buffer_size)
				<< ", Buffer count: " << std::to_string(shm_ptr_->buffer_count) << TLOG_ENDL;
			return;
		}
		else
		{
			TLOG(TLVL_ERROR) << "Failed to attach to shared memory segment "
				<< shm_segment_id_ << TLOG_ENDL;
		}
	}
	else
	{
		TLOG(TLVL_ERROR) << "Failed to connect to shared memory segment"
			<< ", errno = " << strerror(errno) << ".  Please check "
			<< "if a stale shared memory segment needs to "
			<< "be cleaned up. (ipcs, ipcrm -m <segId>)" << TLOG_ENDL;
	}
	return;
}

int artdaq::SharedMemoryManager::GetBufferForReading()
{
	TLOG(13) << "GetBufferForReading BEGIN" << TLOG_ENDL;

	//std::unique_lock<std::mutex> lk(search_mutex_);
	TraceLock lk(search_mutex_, 11, "GetBufferForReadingSearch");
	auto rp = shm_ptr_->reader_pos.load();

	TLOG(13) << "GetBufferForReading lock acquired, scanning buffers" << TLOG_ENDL;
	bool retry = true;
	int buffer_num = -1;
	while (retry)
	{
		ShmBuffer* buffer_ptr = nullptr;
		uint64_t seqID = -1;
		for (auto ii = 0; ii < shm_ptr_->buffer_count; ++ii)
		{
			auto buffer = (ii + rp) % shm_ptr_->buffer_count;


			TLOG(14) << "GetBufferForReading Checking if buffer " << buffer << " is stale" << TLOG_ENDL;
			ResetBuffer(buffer);

			auto buf = getBufferInfo_(buffer);
			TLOG(14) << "GetBufferForReading: Buffer " << buffer << ": sem=" << FlagToString(buf->sem) << " (expected " << FlagToString(BufferSemaphoreFlags::Full) << "), sem_id=" << buf->sem_id << " )" << TLOG_ENDL;
			if (buf->sem == BufferSemaphoreFlags::Full && (buf->sem_id == -1 || buf->sem_id == manager_id_) && buf->sequence_id > last_seen_id_)
			{
				if (buf->sequence_id < seqID)
				{
					buffer_ptr = buf;
					seqID = buf->sequence_id;
					buffer_num = buffer;
				}
			}
		}

		if (!buffer_ptr || (buffer_ptr && buffer_ptr->sem_id != -1 && buffer_ptr->sem_id != manager_id_))
		{
			continue;
		}

		if (buffer_num >= 0)
		{
			TLOG(13) << "GetBufferForReading Found buffer " << buffer_num << TLOG_ENDL;
			buffer_ptr->sem_id = manager_id_;
			buffer_ptr->sem = BufferSemaphoreFlags::Reading;
			if (buffer_ptr->sem_id != manager_id_) { continue; } // Failed to acquire buffer
			buffer_ptr->readPos = 0;
			touchBuffer_(buffer_ptr);
			if (buffer_ptr->sem_id != manager_id_) { continue; } // Failed to acquire buffer
			if (shm_ptr_->destructive_read_mode && shm_ptr_->lowest_seq_id_read == last_seen_id_)
			{
				shm_ptr_->lowest_seq_id_read = seqID;
			}
			last_seen_id_ = seqID;
			if (shm_ptr_->destructive_read_mode) shm_ptr_->reader_pos = (buffer_num + 1) % shm_ptr_->buffer_count;
		}
		retry = false;
	}

	TLOG(13) << "GetBufferForReading returning -1 because no buffers are ready" << TLOG_ENDL;
	return buffer_num;
}

int artdaq::SharedMemoryManager::GetBufferForWriting(bool overwrite)
{
	TLOG(14) << "GetBufferForWriting BEGIN" << TLOG_ENDL;
	//std::unique_lock<std::mutex> lk(search_mutex_);
	TraceLock lk(search_mutex_, 12, "GetBufferForWritingSearch");
	auto wp = shm_ptr_->writer_pos.load();

	// First, only look for "Empty" buffers
	for (auto ii = 0; ii < shm_ptr_->buffer_count; ++ii)
	{
		auto buffer = (ii + wp) % shm_ptr_->buffer_count;

		ResetBuffer(buffer);

		auto buf = getBufferInfo_(buffer);
		if (buf->sem == BufferSemaphoreFlags::Empty)
		{
			buf->sem_id = manager_id_;
			buf->sem = BufferSemaphoreFlags::Writing;
			if (buf->sem_id != manager_id_) continue;
			buf->sequence_id = ++shm_ptr_->next_sequence_id;
			buf->writePos = 0;
			shm_ptr_->writer_pos = (buffer + 1) % shm_ptr_->buffer_count;
			touchBuffer_(buf);
			TLOG(14) << "GetBufferForWriting returning " << buffer << TLOG_ENDL;
			return buffer;
		}
	}

	if (overwrite)
	{
		// Then, look for "Full" buffers
		for (auto ii = 0; ii < shm_ptr_->buffer_count; ++ii)
		{
			auto buffer = (ii + wp) % shm_ptr_->buffer_count;

			ResetBuffer(buffer);

			auto buf = getBufferInfo_(buffer);
			if (buf->sem == BufferSemaphoreFlags::Full)
			{
				buf->sem_id = manager_id_;
				buf->sem = BufferSemaphoreFlags::Writing;
				if (buf->sem_id != manager_id_) continue;
				buf->sequence_id = ++shm_ptr_->next_sequence_id;
				buf->writePos = 0;
				shm_ptr_->writer_pos = (buffer + 1) % shm_ptr_->buffer_count;
				touchBuffer_(buf);
				TLOG(14) << "GetBufferForWriting returning " << buffer << TLOG_ENDL;
				return buffer;
			}
		}

		// Finally, if we still haven't found a buffer, we have to clobber a reader...
		for (auto ii = 0; ii < shm_ptr_->buffer_count; ++ii)
		{
			auto buffer = (ii + wp) % shm_ptr_->buffer_count;

			ResetBuffer(buffer);

			auto buf = getBufferInfo_(buffer);
			if (buf->sem == BufferSemaphoreFlags::Reading)
			{
				buf->sem_id = manager_id_;
				buf->sem = BufferSemaphoreFlags::Writing;
				if (buf->sem_id != manager_id_) continue;
				buf->sequence_id = ++shm_ptr_->next_sequence_id;
				buf->writePos = 0;
				shm_ptr_->writer_pos = (buffer + 1) % shm_ptr_->buffer_count;
				TLOG(14) << "GetBufferForWriting returning " << buffer << TLOG_ENDL;
				touchBuffer_(buf);
				return buffer;
			}
		}
	}

	TLOG(14) << "GetBufferForWriting Returning -1 because no buffers are ready" << TLOG_ENDL;
	return -1;
}

size_t artdaq::SharedMemoryManager::ReadReadyCount()
{
	if (!IsValid()) return 0;
	TLOG(23) << "ReadReadyCount BEGIN" << TLOG_ENDL;
	//std::unique_lock<std::mutex> lk(search_mutex_);
	TraceLock lk(search_mutex_, 14, "ReadReadyCountSearch");
	size_t count = 0;
	for (auto ii = 0; ii < shm_ptr_->buffer_count; ++ii)
	{
		TLOG(24) << "ReadReadyCount: Checking if buffer " << ii << " is stale." << TLOG_ENDL;
		ResetBuffer(ii);
		auto buf = getBufferInfo_(ii);
		TLOG(24) << "ReadReadyCount: Buffer " << ii << ": sem=" << FlagToString(buf->sem) << " (expected " << FlagToString(BufferSemaphoreFlags::Full) << "), sem_id=" << buf->sem_id << " )" << TLOG_ENDL;
		if (buf->sem == BufferSemaphoreFlags::Full && (buf->sem_id == -1 || buf->sem_id == manager_id_) && buf->sequence_id > last_seen_id_)
		{
			TLOG(24) << "ReadReadyCount: Buffer " << ii << " is either unowned or owned by this manager, and is marked full." << TLOG_ENDL;
			++count;
		}
	}

	TLOG(23) << TLOG_ENDL;
	return count;
}

size_t artdaq::SharedMemoryManager::WriteReadyCount(bool overwrite)
{
	if (!IsValid()) return 0;
	//std::unique_lock<std::mutex> lk(search_mutex_);
	TraceLock lk(search_mutex_, 15, "WriteReadyCountSearch");
	size_t count = 0;
	for (auto ii = 0; ii < shm_ptr_->buffer_count; ++ii)
	{
		ResetBuffer(ii);
		auto buf = getBufferInfo_(ii);
		if ((buf->sem == BufferSemaphoreFlags::Empty && buf->sem_id == -1)
			|| (overwrite && buf->sem != BufferSemaphoreFlags::Writing))
		{
			++count;
		}
	}
	return count;
}

std::deque<int> artdaq::SharedMemoryManager::GetBuffersOwnedByManager()
{
	//std::unique_lock<std::mutex> lk(search_mutex_);
	std::deque<int> output;
	if (!IsValid()) return output;
	TraceLock lk(search_mutex_, 16, "GetOwnedSearch");
	for (auto ii = 0; ii < shm_ptr_->buffer_count; ++ii)
	{
		auto buf = getBufferInfo_(ii);
		if (buf->sem_id == manager_id_)
		{
			output.push_back(ii);
		}
	}

	return output;
}

size_t artdaq::SharedMemoryManager::BufferDataSize(int buffer)
{
	//std::unique_lock<std::mutex> lk(buffer_mutexes_[buffer]);
	TraceLock lk(buffer_mutexes_[buffer], 17, "DataSizeBuffer" + std::to_string(buffer));
	auto buf = getBufferInfo_(buffer);
	touchBuffer_(buf);
	return buf->writePos;
}


void artdaq::SharedMemoryManager::ResetReadPos(int buffer)
{
	//std::unique_lock<std::mutex> lk(buffer_mutexes_[buffer]);
	TraceLock lk(buffer_mutexes_[buffer], 18, "ResetReadPosBuffer" + std::to_string(buffer));
	auto buf = getBufferInfo_(buffer);
	touchBuffer_(buf);
	buf->readPos = 0;
}

void artdaq::SharedMemoryManager::ResetWritePos(int buffer)
{
	//std::unique_lock<std::mutex> lk(buffer_mutexes_[buffer]);
	TraceLock lk(buffer_mutexes_[buffer], 18, "ResetWritePosBuffer" + std::to_string(buffer));
	auto buf = getBufferInfo_(buffer);
	checkBuffer_(buf, BufferSemaphoreFlags::Writing);
	touchBuffer_(buf);
	buf->writePos = 0;
}

void artdaq::SharedMemoryManager::IncrementReadPos(int buffer, size_t read)
{
	//std::unique_lock<std::mutex> lk(buffer_mutexes_[buffer]);
	TraceLock lk(buffer_mutexes_[buffer], 19, "IncReadPosBuffer" + std::to_string(buffer));
	auto buf = getBufferInfo_(buffer);
	touchBuffer_(buf);
	TLOG(15) << "IncrementReadPos: buffer= " << buffer << ", readPos=" << std::to_string(buf->readPos) << ", bytes read=" << std::to_string(read) << TLOG_ENDL;
	buf->readPos = buf->readPos + read;
	TLOG(15) << "IncrementReadPos: buffer= " << buffer << ", New readPos is " << std::to_string(buf->readPos) << TLOG_ENDL;
	if (read == 0)	Detach(true, "LogicError", "Cannot increment Read pos by 0! (buffer=" + std::to_string(buffer) + ", readPos=" + std::to_string(buf->readPos) + ", writePos=" + std::to_string(buf->writePos) + ")");
}

void artdaq::SharedMemoryManager::IncrementWritePos(int buffer, size_t written)
{
	//std::unique_lock<std::mutex> lk(buffer_mutexes_[buffer]);
	TraceLock lk(buffer_mutexes_[buffer], 20, "IncWritePosBuffer" + std::to_string(buffer));
	auto buf = getBufferInfo_(buffer);
	touchBuffer_(buf);
	TLOG(16) << "IncrementWritePos: buffer= " << buffer << ", writePos=" << std::to_string(buf->writePos) << ", bytes written=" << std::to_string(written) << TLOG_ENDL;
	buf->writePos += written;
	TLOG(16) << "IncrementWritePos: buffer= " << buffer << ", New writePos is " << std::to_string(buf->writePos) << TLOG_ENDL;
	if (written == 0)  Detach(true, "LogicError", "Cannot increment Write pos by 0!");
}

bool artdaq::SharedMemoryManager::MoreDataInBuffer(int buffer)
{
	//std::unique_lock<std::mutex> lk(buffer_mutexes_[buffer]);
	TraceLock lk(buffer_mutexes_[buffer], 21, "MoreDataInBuffer" + std::to_string(buffer));
	auto buf = getBufferInfo_(buffer);
	TLOG(17) << "MoreDataInBuffer: buffer= " << buffer << ", readPos=" << std::to_string(buf->readPos) << ", writePos=" << std::to_string(buf->writePos) << TLOG_ENDL;
	return buf->readPos < buf->writePos;
}

bool artdaq::SharedMemoryManager::CheckBuffer(int buffer, BufferSemaphoreFlags flags)
{
	//std::unique_lock<std::mutex> lk(buffer_mutexes_[buffer]);
	TraceLock lk(buffer_mutexes_[buffer], 22, "CheckBuffer" + std::to_string(buffer));
	return checkBuffer_(getBufferInfo_(buffer), flags, false);
}

void artdaq::SharedMemoryManager::MarkBufferFull(int buffer, int destination)
{
	//std::unique_lock<std::mutex> lk(buffer_mutexes_[buffer]);
	TraceLock lk(buffer_mutexes_[buffer], 23, "FillBuffer" + std::to_string(buffer));
	auto shmBuf = getBufferInfo_(buffer);
	touchBuffer_(shmBuf);
	if (shmBuf->sem_id == manager_id_)
	{
		if (shmBuf->sem != BufferSemaphoreFlags::Full)
			shmBuf->sem = BufferSemaphoreFlags::Full;

		shmBuf->sem_id = destination;
	}
}

void artdaq::SharedMemoryManager::MarkBufferEmpty(int buffer, bool force)
{
	TLOG(18) << "MarkBufferEmpty BEGIN" << TLOG_ENDL;
	//std::unique_lock<std::mutex> lk(buffer_mutexes_[buffer]);
	TraceLock lk(buffer_mutexes_[buffer], 24, "EmptyBuffer" + std::to_string(buffer));
	auto shmBuf = getBufferInfo_(buffer);
	if (!force)
	{
		checkBuffer_(shmBuf, BufferSemaphoreFlags::Reading, true);
	}
	touchBuffer_(shmBuf);

	shmBuf->readPos = 0;
	shmBuf->sem = BufferSemaphoreFlags::Full;

	if ((force && (manager_id_ == 0 || manager_id_ == shmBuf->sem_id)) || (!force && shm_ptr_->destructive_read_mode))
	{
		TLOG(18) << "MarkBufferEmpty Resetting buffer to Empty state" << TLOG_ENDL;
		shmBuf->writePos = 0;
		shmBuf->sem = BufferSemaphoreFlags::Empty;
		if (shm_ptr_->reader_pos == static_cast<unsigned>(buffer) && !shm_ptr_->destructive_read_mode)
		{
			TLOG(18) << "MarkBufferEmpty Broadcast mode; incrementing reader_pos" << TLOG_ENDL;
			shm_ptr_->reader_pos = (buffer + 1) % shm_ptr_->buffer_count;
		}
	}
	shmBuf->sem_id = -1;
	TLOG(18) << "MarkBufferEmpty END" << TLOG_ENDL;
}

bool artdaq::SharedMemoryManager::ResetBuffer(int buffer)
{
	//std::unique_lock<std::mutex> lk(buffer_mutexes_[buffer]);
	TraceLock lk(buffer_mutexes_[buffer], 25, "ResetBuffer" + std::to_string(buffer));
	auto shmBuf = getBufferInfo_(buffer);
	/*
		if (shmBuf->sequence_id < shm_ptr_->lowest_seq_id_read - size() && shmBuf->sem == BufferSemaphoreFlags::Full)
		{
			TLOG(TLVL_DEBUG) << "Buffer " << buffer << " has been passed by all readers, marking Empty" << TLOG_ENDL;
			shmBuf->writePos = 0;
			shmBuf->sem = BufferSemaphoreFlags::Empty;
			shmBuf->sem_id = -1;
			return true;
		}*/

	size_t delta = TimeUtils::gettimeofday_us() - shmBuf->last_touch_time;
	if (delta > 0xFFFFFFFF) {
		TLOG(TLVL_TRACE) << "Buffer has touch time in the future, setting it to current time and ignoring..." << TLOG_ENDL;
		shmBuf->last_touch_time = TimeUtils::gettimeofday_us();
		return false;
	}
	if (delta <= shm_ptr_->buffer_timeout_us || shmBuf->sem == BufferSemaphoreFlags::Empty) return false;
	TLOG(TLVL_TRACE) << "Buffer " << buffer << " is stale, time=" << TimeUtils::gettimeofday_us() << ", last touch=" << shmBuf->last_touch_time << ", d=" << delta << ", timeout=" << shm_ptr_->buffer_timeout_us << TLOG_ENDL;

	if (shmBuf->sem_id == manager_id_ && shmBuf->sem == BufferSemaphoreFlags::Writing)
	{
		return true;
	}
	if (!shm_ptr_->destructive_read_mode && shmBuf->sem == BufferSemaphoreFlags::Full)
	{
		TLOG(TLVL_DEBUG) << "Resetting old broadcast mode buffer" << TLOG_ENDL;
		shmBuf->writePos = 0;
		shmBuf->sem = BufferSemaphoreFlags::Empty;
		shmBuf->sem_id = -1;
		if (shm_ptr_->reader_pos == static_cast<unsigned>(buffer)) shm_ptr_->reader_pos = (buffer + 1) % shm_ptr_->buffer_count;
		return true;
	}

	if (shmBuf->sem_id != manager_id_ && shmBuf->sem == BufferSemaphoreFlags::Reading)
	{
		TLOG(TLVL_WARNING) << "Stale Read buffer ( " << delta << " / " << shm_ptr_->buffer_timeout_us << " us ) detected! Resetting..." << TLOG_ENDL;
		shmBuf->readPos = 0;
		shmBuf->sem = BufferSemaphoreFlags::Full;
		shmBuf->sem_id = -1;
		return true;
	}
	return false;
}

bool artdaq::SharedMemoryManager::IsEndOfData() const
{
	if (!IsValid()) return true;

	struct shmid_ds info;
	auto sts = shmctl(shm_segment_id_, IPC_STAT, &info);
	if (sts < 0) {
		TLOG(TLVL_TRACE) << "Error accessing Shared Memory info: " << errno << " (" << strerror(errno) << ").";
		return true;
	}

	if (info.shm_perm.mode & SHM_DEST) {
		TLOG(TLVL_INFO) << "Shared Memory marked for destruction. Probably an end-of-data condition!";
		return true;
	}

	return false;
}

size_t artdaq::SharedMemoryManager::Write(int buffer, void* data, size_t size)
{
	TLOG(19) << "Write BEGIN" << TLOG_ENDL;
	//std::unique_lock<std::mutex> lk(buffer_mutexes_[buffer]);
	TraceLock lk(buffer_mutexes_[buffer], 26, "WriteBuffer" + std::to_string(buffer));
	auto shmBuf = getBufferInfo_(buffer);
	checkBuffer_(shmBuf, BufferSemaphoreFlags::Writing);
	touchBuffer_(shmBuf);
	TLOG(19) << "Buffer Write Pos is " << std::to_string(shmBuf->writePos) << ", write size is " << std::to_string(size) << TLOG_ENDL;
	if (shmBuf->writePos + size > shm_ptr_->buffer_size) Detach(true, "SharedMemoryWrite", "Attempted to write more data than fits into Shared Memory! \nRe-run with a larger buffer size!");

	auto pos = GetWritePos(buffer);
	memcpy(pos, data, size);
	shmBuf->writePos = shmBuf->writePos + size;
	if (shmBuf->sequence_id > last_seen_id_)
	{
		last_seen_id_ = shmBuf->sequence_id;
	}
	TLOG(19) << "Write END" << TLOG_ENDL;
	return size;
}

bool artdaq::SharedMemoryManager::Read(int buffer, void* data, size_t size)
{
	//std::unique_lock<std::mutex> lk(buffer_mutexes_[buffer]);
	TraceLock lk(buffer_mutexes_[buffer], 27, "ReadBuffer" + std::to_string(buffer));
	auto shmBuf = getBufferInfo_(buffer);
	checkBuffer_(shmBuf, BufferSemaphoreFlags::Reading);
	touchBuffer_(shmBuf);
	if (shmBuf->readPos + size > shm_ptr_->buffer_size)  Detach(true, "SharedMemoryRead", "Attempted to read more data than exists in Shared Memory!");

	auto pos = GetReadPos(buffer);
	memcpy(data, pos, size);
	shmBuf->readPos += size;
	touchBuffer_(shmBuf);
	return checkBuffer_(shmBuf, BufferSemaphoreFlags::Reading, false);
}

std::string artdaq::SharedMemoryManager::toString()
{
	std::ostringstream ostr;
	ostr << "ShmStruct: " << std::endl
		<< "Reader Position: " << shm_ptr_->reader_pos << std::endl
		<< "Writer Position: " << shm_ptr_->writer_pos << std::endl
		<< "Next ID Number: " << shm_ptr_->next_id << std::endl
		<< "Buffer Count: " << shm_ptr_->buffer_count << std::endl
		<< "Buffer Size: " << std::to_string(shm_ptr_->buffer_size) << " bytes" << std::endl
		<< "Buffers Written: " << std::to_string(shm_ptr_->next_sequence_id) << std::endl
		<< "Rank of Writer: " << shm_ptr_->rank << std::endl
		<< "Ready Magic Bytes: 0x" << std::hex << shm_ptr_->ready_magic << std::endl << std::endl;

	for (auto ii = 0; ii < shm_ptr_->buffer_count; ++ii)
	{
		auto buf = getBufferInfo_(ii);
		ostr << "ShmBuffer " << std::dec << ii << std::endl
			<< "sequenceID: " << std::to_string(buf->sequence_id) << std::endl
			<< "writePos: " << std::to_string(buf->writePos) << std::endl
			<< "readPos: " << std::to_string(buf->readPos) << std::endl
			<< "sem: " << FlagToString(buf->sem) << std::endl
			<< "Owner: " << std::to_string(buf->sem_id.load()) << std::endl
			<< "Last Touch Time: " << std::to_string(buf->last_touch_time / 1000000.0) << std::endl << std::endl;
	}

	return ostr.str();
}

void* artdaq::SharedMemoryManager::GetReadPos(int buffer)
{
	auto buf = getBufferInfo_(buffer);
	return bufferStart_(buffer) + buf->readPos;
}
void* artdaq::SharedMemoryManager::GetWritePos(int buffer)
{
	auto buf = getBufferInfo_(buffer);
	return bufferStart_(buffer) + buf->writePos;
}

void* artdaq::SharedMemoryManager::GetBufferStart(int buffer)
{
	return bufferStart_(buffer);
}

uint8_t* artdaq::SharedMemoryManager::dataStart_() const
{
	return reinterpret_cast<uint8_t*>(shm_ptr_ + 1) + shm_ptr_->buffer_count * sizeof(ShmBuffer);
}

uint8_t* artdaq::SharedMemoryManager::bufferStart_(int buffer)
{
	if (buffer >= shm_ptr_->buffer_count)  Detach(true, "ArgumentOutOfRange", "The specified buffer does not exist!");
	return dataStart_() + buffer * shm_ptr_->buffer_size;
}

artdaq::SharedMemoryManager::ShmBuffer* artdaq::SharedMemoryManager::getBufferInfo_(int buffer)
{
	if (buffer >= shm_ptr_->buffer_count)  Detach(true, "ArgumentOutOfRange", "The specified buffer does not exist!");
	return reinterpret_cast<ShmBuffer*>(reinterpret_cast<uint8_t*>(shm_ptr_ + 1) + buffer * sizeof(ShmBuffer));
}

bool artdaq::SharedMemoryManager::checkBuffer_(ShmBuffer* buffer, BufferSemaphoreFlags flags, bool exceptions)
{
	if (exceptions)
	{
		if (buffer->sem != flags) Detach(true, "StateAccessViolation", "Shared Memory buffer is not in the correct state! (expected " + FlagToString(flags) + ", actual " + FlagToString(buffer->sem) + ")");
		if (buffer->sem_id != manager_id_)  Detach(true, "OwnerAccessViolation", "Shared Memory buffer is not owned by this manager instance!");
	}
	bool ret = (buffer->sem_id == manager_id_ || (buffer->sem_id == -1 && (flags == BufferSemaphoreFlags::Full || flags == BufferSemaphoreFlags::Empty))) && buffer->sem == flags;

	if (!ret)
	{
		TLOG(TLVL_WARNING) << "CheckBuffer detected issue with buffer " << std::to_string(buffer->sequence_id) << "!"
			<< " ID: " << buffer->sem_id << " (" << manager_id_ << "), Flag: " << FlagToString(buffer->sem) << " (" << FlagToString(flags) << "). "
			<< "ID -1 is okay if desired flag is \"Full\" or \"Empty\"." << TLOG_ENDL;
	}

	return ret;
}

void artdaq::SharedMemoryManager::touchBuffer_(ShmBuffer* buffer)
{
	if (buffer->sem_id != manager_id_) return;
	TLOG(TLVL_TRACE) << "touchBuffer_: Touching buffer with sequence_id " << std::to_string(buffer->sequence_id) << TLOG_ENDL;
	buffer->last_touch_time = TimeUtils::gettimeofday_us();
}

void artdaq::SharedMemoryManager::Detach(bool throwException, std::string category, std::string message, bool force)
{
	TLOG(TLVL_DETACH) << "Detach BEGIN: throwException: " << std::boolalpha << throwException << ", force: " << force;
	if (IsValid())
	{
		TLOG(TLVL_DETACH) << "Detach: Resetting owned buffers";
		auto bufs = GetBuffersOwnedByManager();
		for (auto buf : bufs)
		{
			auto shmBuf = getBufferInfo_(buf);
			if (shmBuf->sem == BufferSemaphoreFlags::Writing)
			{
				shmBuf->sem = BufferSemaphoreFlags::Empty;
			}
			else if (shmBuf->sem == BufferSemaphoreFlags::Reading)
			{
				shmBuf->sem = BufferSemaphoreFlags::Full;
			}
			shmBuf->sem_id = -1;
		}
	}

	if (shm_ptr_)
	{
		TLOG(TLVL_DETACH) << "Detach: Detaching shared memory";
		shmdt(shm_ptr_);
		shm_ptr_ = NULL;
	}

	if ((force || manager_id_ == 0) && shm_segment_id_ > -1)
	{
		TLOG(TLVL_DETACH) << "Detach: Marking Shared memory for removal";
		shmctl(shm_segment_id_, IPC_RMID, NULL);
		shm_segment_id_ = -1;
	}

	if (category.size() > 0 && message.size() > 0)
	{
		TLOG(TLVL_ERROR) << category << ": " << message << TLOG_ENDL;

		if (throwException)
		{
			throw cet::exception(category) << message;
		}
	}
}



// Local Variables:
// mode: c++
// End:
