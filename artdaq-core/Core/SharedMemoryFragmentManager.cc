#include "artdaq-core/Core/SharedMemoryFragmentManager.hh"
#include "tracemf.h"


artdaq::SharedMemoryFragmentManager::SharedMemoryFragmentManager(int shm_key, size_t buffer_count, size_t max_buffer_size)
	: SharedMemoryManager(shm_key, buffer_count, max_buffer_size)
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
	while(sts < hdrSize)
	{
		auto res = Read(buf, reinterpret_cast<uint8_t*>(fragment.headerAddress()) + sts, hdrSize - sts);
		if(res == 0)
		{
			ReleaseBuffer(buf);
			return -1;
		}
		sts += res;
	}

	fragment.autoResize(); 
	sts = Read(buf, fragment.headerAddress() + hdrSize, hdrSize) + hdrSize;
	while (sts < fragment.sizeBytes())
	{
		auto res = Read(buf, reinterpret_cast<uint8_t*>(fragment.headerAddress()) + sts, fragment.sizeBytes() - sts);
		if (res == 0)
		{
			ReleaseBuffer(buf);
			return -1;
		}
		sts += res;
	}

	MarkBufferEmpty(buf);
	return 0;
}

