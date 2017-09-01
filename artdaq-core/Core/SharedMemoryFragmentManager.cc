#include "artdaq-core/Core/SharedMemoryFragmentManager.hh"
#include "tracemf.h"


artdaq::SharedMemoryFragmentManager::SharedMemoryFragmentManager(uint32_t shm_key, size_t buffer_count, size_t max_buffer_size, size_t buffer_timeout_us)
	: SharedMemoryManager(shm_key, buffer_count, max_buffer_size, buffer_timeout_us)
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
		TLOG_ARB(13, "SharedMemoryFragmentManager") << "Done sending Fragment with seqID=" << fragment.sequenceID() << TLOG_ENDL;
		MarkBufferFull(buf);
		return 0;
	}
	TLOG_ERROR("SharedMemoryFragmentManager") << "Unexpected status from SharedMemory Write call!" << TLOG_ENDL;
	return -2;
}

int artdaq::SharedMemoryFragmentManager::ReadFragment(Fragment& fragment)
{
	TLOG_ARB(13, "SharedMemoryFragmentManager") << "ReadFragment BEGIN" << TLOG_ENDL;
	detail::RawFragmentHeader tmpHdr;

	TLOG_ARB(13, "SharedMemoryFragmentManager") << "Reading Fragment Header" << TLOG_ENDL;
	auto sts = ReadFragmentHeader(tmpHdr);
	if (sts != 0) return sts;
	fragment.resize(tmpHdr.word_count - tmpHdr.num_words());
	memcpy(fragment.headerAddress(), &tmpHdr, tmpHdr.num_words() * sizeof(artdaq::RawDataType));
	TLOG_ARB(13, "SharedMemoryFragmentManager") << "Reading Fragment Body" << TLOG_ENDL;
	return ReadFragmentData(fragment.headerAddress() + tmpHdr.num_words(), tmpHdr.word_count - tmpHdr.num_words());
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
	if (!IsValid() || active_buffer_ == -1 || !CheckBuffer(active_buffer_, BufferSemaphoreFlags::Reading)) {
		TLOG_ERROR("SharedMemoryFragmentManager") << "ReadFragmentData: Buffer " << active_buffer_ << " failed status checks: IsValid()=" << std::boolalpha << IsValid() << ", CheckBuffer=" << CheckBuffer(active_buffer_, BufferSemaphoreFlags::Reading) << TLOG_ENDL;
		return -3;
	}
	
	auto sts = Read(active_buffer_, destination, words * sizeof(RawDataType));
	if (!sts) {
		TLOG_ERROR("SharedMemoryFragmentManager") << "ReadFragmentData: Buffer " << active_buffer_ << " returned bad status code from Read" << TLOG_ENDL;
		return -2;
	}

	MarkBufferEmpty(active_buffer_);
	active_buffer_ = -1;
	return 0;
}
