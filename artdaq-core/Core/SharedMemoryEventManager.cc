#include "artdaq-core/Core/SharedMemoryEventManager.hh"
#include <set>

artdaq::SharedMemoryEventManager::SharedMemoryEventManager(int shm_key, size_t buffer_count, size_t max_buffer_size, size_t fragment_count)
	: SharedMemoryManager(shm_key,buffer_count,max_buffer_size)
, fragments_per_complete_event_(fragment_count)
, current_read_buffer_(-1)
{
	
}

std::shared_ptr<artdaq::detail::RawEventHeader> artdaq::SharedMemoryEventManager::ReadHeader()
{
	if (current_header_) return current_header_;
	auto buf = GetBufferForReading();
	if (buf == -1) throw cet::exception("OutOfEvents") << "ReadHeader called but no events are ready! (Did you check ReadyForRead()?)";
	current_read_buffer_ = buf;
	ResetReadPos(current_read_buffer_);
	current_header_ = std::shared_ptr<detail::RawEventHeader>(reinterpret_cast<detail::RawEventHeader*>(GetReadPos(buf)));
	return current_header_;
}

std::set<artdaq::Fragment::type_t> artdaq::SharedMemoryEventManager::GetFragmentTypes()
{
	if (current_read_buffer_ == -1) throw cet::exception("AccessViolation") << "Cannot call GetFragmentTypes when not currently reading a buffer! Call ReadHeader() first!";
	ResetReadPos(current_read_buffer_);
	IncrementReadPos(current_read_buffer_, sizeof(detail::RawEventHeader));
	
	auto output = std::set<Fragment::type_t>();

	while (MoreDataInBuffer(current_read_buffer_))
	{
		auto fragHdr = reinterpret_cast<artdaq::detail::RawFragmentHeader*>(GetReadPos(current_read_buffer_));
		output.insert(fragHdr->type);
		IncrementReadPos(current_read_buffer_,fragHdr->word_count * sizeof(RawDataType));
	}

	return output;
}

std::unique_ptr<artdaq::Fragments> artdaq::SharedMemoryEventManager::GetFragmentsByType(Fragment::type_t type)
{
	if (current_read_buffer_ == -1) throw cet::exception("AccessViolation") << "Cannot call GetFragmentsByType when not currently reading a buffer! Call ReadHeader() first!";
	ResetReadPos(current_read_buffer_);
	IncrementReadPos(current_read_buffer_, sizeof(detail::RawEventHeader));

	Fragments output;

	while (MoreDataInBuffer(current_read_buffer_))
	{
		auto fragHdr = reinterpret_cast<artdaq::detail::RawFragmentHeader*>(GetReadPos(current_read_buffer_));
		if (fragHdr->type != type) continue;

		output.emplace_back(fragHdr->word_count - detail::RawFragmentHeader::num_words());
		memcpy(output.back().headerAddress(), GetReadPos(current_read_buffer_), fragHdr->word_count * sizeof(RawDataType));

		IncrementReadPos(current_read_buffer_, fragHdr->word_count * sizeof(RawDataType));
	}

	return std::unique_ptr<Fragments>(&output);
}

void artdaq::SharedMemoryEventManager::ReleaseBuffer()
{
	SharedMemoryManager::ReleaseBuffer(current_read_buffer_);
	current_read_buffer_ = -1;
	current_header_.reset();
}