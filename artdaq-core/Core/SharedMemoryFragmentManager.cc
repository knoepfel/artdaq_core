#include "artdaq-core/Core/SharedMemoryFragmentManager.hh"
#include "tracemf.h"


artdaq::SharedMemoryFragmentManager::SharedMemoryFragmentManager(int shm_key, size_t buffer_count, size_t max_buffer_size, size_t stale_buffer_touch_count)
	: SharedMemoryManager(shm_key, buffer_count, max_buffer_size, stale_buffer_touch_count)
, active_buffer_(-1)
{

}

int artdaq::SharedMemoryFragmentManager::WriteFragment(Fragment&& fragment, bool overwrite)
{
	if (!IsValid()) { return -1; }

	TLOG_ARB(13, "SharedMemoryFragmentManager") << "Sending fragment with seqID=" << fragment.sequenceID() << TLOG_ENDL;
	artdaq::RawDataType* fragAddr = fragment.headerAddress();
	size_t fragSize = fragment.size() * sizeof(artdaq::RawDataType);

	auto buf = GetBufferForWriting(overwrite);
	auto sts = Write(buf, fragAddr, fragSize);
	if(sts == fragSize)
	{
		MarkBufferFull(buf);
		return 0;
	}
	return -2;
}

int artdaq::SharedMemoryFragmentManager::ReadFragment(Fragment& fragment)
{
	if (!IsValid()) return -1;

	size_t hdrSize = artdaq::detail::RawFragmentHeader::num_words() * sizeof(artdaq::RawDataType);
	fragment.resizeBytes(0);
	auto buf = GetBufferForReading();

	auto sts = Read(buf, fragment.headerAddress(), hdrSize);
	if (!sts) return -1;

	fragment.autoResize(); 
	sts = Read(buf, fragment.headerAddress() + hdrSize, hdrSize) + hdrSize;
	if (!sts) return -1;

	MarkBufferEmpty(buf);
	return 0;
}

int artdaq::SharedMemoryFragmentManager::ReadFragmentHeader(detail::RawFragmentHeader& header)
{
	if (!IsValid()) return -3;

	size_t hdrSize = artdaq::detail::RawFragmentHeader::num_words() * sizeof(artdaq::RawDataType);
	active_buffer_ = GetBufferForReading();

	if (active_buffer_ == -1) return -1;

	auto sts = Read(active_buffer_, &header, hdrSize);
	if (!sts) return -2;

	return 0;
}

int artdaq::SharedMemoryFragmentManager::ReadFragmentData(RawDataType* destination, size_t words)
{
	if (!IsValid() || active_buffer_ == -1 || !CheckBuffer(active_buffer_, BufferSemaphoreFlags::Reading)) return -3;
	
	auto sts = Read(active_buffer_, destination, words * sizeof(RawDataType));
	if (!sts) return -2;

	MarkBufferEmpty(active_buffer_);
	active_buffer_ = -1;
	return 0;
}

