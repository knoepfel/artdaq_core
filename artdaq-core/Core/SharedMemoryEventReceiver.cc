
#include "artdaq-core/Core/SharedMemoryEventReceiver.hh"

#include "artdaq-core/Data/Fragment.hh"
#include <sys/time.h>
#define TRACE_NAME "SharedMemoryEventReceiver"
#include "tracemf.h"

using std::string;

artdaq::SharedMemoryEventReceiver::SharedMemoryEventReceiver(uint32_t shm_key)
	: SharedMemoryManager(shm_key)
	, current_read_buffer_(-1)
	, current_header_(nullptr)
{
	TLOG_TRACE("SharedMemoryEventReceiver") << "SharedMemoryEventReceiver CONSTRUCTOR" << TLOG_ENDL;
}

artdaq::detail::RawEventHeader* artdaq::SharedMemoryEventReceiver::ReadHeader(bool& err, BufferMode mode)
{
	if (current_read_buffer_ != -1) {
		err = !CheckBuffer(current_read_buffer_, BufferSemaphoreFlags::Reading);
		if (err) return nullptr;
	}
	if (current_header_)
	{
		TLOG_TRACE("SharedMemoryEventReceiver") << "Already have buffer, returning stored header" << TLOG_ENDL;
		return current_header_;
	}
	auto buf = GetBufferForReading(mode);
	if (buf == -1) throw cet::exception("OutOfEvents") << "ReadHeader called but no events are ready! (Did you check ReadyForRead()?)";
	current_read_buffer_ = buf;
	ResetReadPos(current_read_buffer_);
	current_header_ = reinterpret_cast<detail::RawEventHeader*>(GetReadPos(buf));
	return current_header_;
}

std::set<artdaq::Fragment::type_t> artdaq::SharedMemoryEventReceiver::GetFragmentTypes(bool& err)
{
	if (current_read_buffer_ == -1) throw cet::exception("AccessViolation") << "Cannot call GetFragmentTypes when not currently reading a buffer! Call ReadHeader() first!";

	err = !CheckBuffer(current_read_buffer_, BufferSemaphoreFlags::Reading);
	if (err) return std::set<Fragment::type_t>();

	ResetReadPos(current_read_buffer_);
	IncrementReadPos(current_read_buffer_, sizeof(detail::RawEventHeader));
	auto output = std::set<Fragment::type_t>();

	while (MoreDataInBuffer(current_read_buffer_))
	{
		err = !CheckBuffer(current_read_buffer_, BufferSemaphoreFlags::Reading);
		if (err) return std::set<Fragment::type_t>();
		auto fragHdr = reinterpret_cast<artdaq::detail::RawFragmentHeader*>(GetReadPos(current_read_buffer_));
		output.insert(fragHdr->type);
		IncrementReadPos(current_read_buffer_, fragHdr->word_count * sizeof(RawDataType));
	}

	return output;
}


std::unique_ptr<artdaq::Fragments> artdaq::SharedMemoryEventReceiver::GetFragmentsByType(bool& err, Fragment::type_t type)
{
	if (current_read_buffer_ == -1) throw cet::exception("AccessViolation") << "Cannot call GetFragmentsByType when not currently reading a buffer! Call ReadHeader() first!";
	err = !CheckBuffer(current_read_buffer_, BufferSemaphoreFlags::Reading);
	if (err) return nullptr;

	ResetReadPos(current_read_buffer_);
	IncrementReadPos(current_read_buffer_, sizeof(detail::RawEventHeader));

	std::unique_ptr<Fragments> output(new Fragments());

	while (MoreDataInBuffer(current_read_buffer_))
	{
		err = !CheckBuffer(current_read_buffer_, BufferSemaphoreFlags::Reading);
		if (err) return nullptr;
		auto fragHdr = reinterpret_cast<artdaq::detail::RawFragmentHeader*>(GetReadPos(current_read_buffer_));
		if (fragHdr->type == type || type == Fragment::InvalidFragmentType) {
			output->emplace_back(fragHdr->word_count - detail::RawFragmentHeader::num_words());
			Read(current_read_buffer_, output->back().headerAddress(), fragHdr->word_count * sizeof(RawDataType));
			output->back().autoResize();
		}
		else {
			IncrementReadPos(current_read_buffer_, fragHdr->word_count * sizeof(RawDataType));
		}
	}

	return std::move(output);
}

std::string artdaq::SharedMemoryEventReceiver::toString()
{
	std::ostringstream ostr;
	ostr << SharedMemoryManager::toString() << std::endl;

	ostr << "Buffer Fragment Counts: " << std::endl;
	for (size_t ii = 0; ii < size(); ++ii)
	{
		ostr << "Buffer " << std::to_string(ii) << ": " << std::endl;

		ResetReadPos(ii);
		IncrementReadPos(ii, sizeof(detail::RawEventHeader));

		while (MoreDataInBuffer(ii))
		{
			auto fragHdr = reinterpret_cast<artdaq::detail::RawFragmentHeader*>(GetReadPos(ii));
			ostr << "    Fragment " << std::to_string(fragHdr->fragment_id) << ": Sequence ID: " << std::to_string(fragHdr->sequence_id) << ", Type:" << std::to_string(fragHdr->type);
			if (fragHdr->MakeVerboseSystemTypeMap().count(fragHdr->type)) {
				ostr << " (" << fragHdr->MakeVerboseSystemTypeMap()[fragHdr->type] << ")";
			}
			ostr << ", Size: " << std::to_string(fragHdr->word_count) << " words." << std::endl;
			IncrementReadPos(ii, fragHdr->word_count * sizeof(RawDataType));
		}

	}
	return ostr.str();
}

void artdaq::SharedMemoryEventReceiver::ReleaseBuffer()
{
	TLOG_TRACE("SharedMemoryEventReceiver") << "ReleaseBuffer BEGIN" << TLOG_ENDL;
	try {
		MarkBufferEmpty(current_read_buffer_);
	}
	catch (cet::exception e)
	{
		TLOG_WARNING("SharedMemoryEventReceiver") << "A cet::exception occured while trying to release the buffer: " << e << TLOG_ENDL;
	}
	current_read_buffer_ = -1;
	current_header_ = nullptr;
	TLOG_TRACE("SharedMemoryEventReceiver") << "ReleaseBuffer END" << TLOG_ENDL;
}
