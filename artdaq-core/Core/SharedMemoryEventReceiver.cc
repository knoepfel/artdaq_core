
#include "artdaq-core/Core/SharedMemoryEventReceiver.hh"

#include "artdaq-core/Data/Fragment.hh"
#include <sys/time.h>
#define TRACE_NAME "SharedMemoryEventReceiver"
#include "tracemf.h"

using std::string;

artdaq::SharedMemoryEventReceiver::SharedMemoryEventReceiver(uint32_t shm_key, uint32_t broadcast_shm_key)
	: current_read_buffer_(-1)
	, initialized_(false)
	, current_header_(nullptr)
	, current_data_source_(nullptr)
	, data_(shm_key)
	, broadcasts_(broadcast_shm_key)
{
	TLOG_TRACE("SharedMemoryEventReceiver") << "SharedMemoryEventReceiver CONSTRUCTOR" << TLOG_ENDL;
}

bool artdaq::SharedMemoryEventReceiver::ReadyForRead(bool broadcast, size_t timeout_us)
{
	TLOG(4) << "ReadyForRead BEGIN" << TLOG_ENDL;
	if (current_read_buffer_ != -1 && current_data_source_ && current_header_)
	{
		TLOG_TRACE("SharedMemoryEventReceiver") << "ReadyForRead Returning true because already reading buffer" << TLOG_ENDL;
		return true;
	}

	bool first = true;
	auto start_time = TimeUtils::gettimeofday_us();
	int buf = -1;
	while (first || TimeUtils::gettimeofday_us() - start_time < timeout_us)
	{
		if (broadcasts_.ReadyForRead())
		{
			buf = broadcasts_.GetBufferForReading();
			current_data_source_ = &broadcasts_;
		}
		else if (!broadcast && data_.ReadyForRead())
		{
			buf = data_.GetBufferForReading();
			current_data_source_ = &data_;
		}
		if (buf != -1 && current_data_source_)
		{
			TLOG_TRACE("SharedMemoryEventReceiver") << "ReadyForRead Found buffer, returning true" << TLOG_ENDL;
			current_read_buffer_ = buf;
			current_data_source_->ResetReadPos(buf);
			current_header_ = reinterpret_cast<detail::RawEventHeader*>(current_data_source_->GetReadPos(buf));

			// Ignore any Init fragments after the first
			if (current_data_source_ == &broadcasts_)
			{
				bool err;
				auto types = GetFragmentTypes(err);
				if (!err && types.count(Fragment::type_t(Fragment::InitFragmentType)) && initialized_)
				{
					ReleaseBuffer();
					continue;
				}
				else if (!err && types.count(Fragment::type_t(Fragment::InitFragmentType)))
				{
					initialized_ = true;
				}
			}

			return true;
		}
		current_data_source_ = nullptr;
		first = false;
		usleep( 100 );
	}
	TLOG(4) << "ReadyForRead returning false" << TLOG_ENDL;
	return false;
}

artdaq::detail::RawEventHeader* artdaq::SharedMemoryEventReceiver::ReadHeader(bool& err)
{
	TLOG_TRACE("SharedMemoryEventReceiver") << "ReadHeader BEGIN" << TLOG_ENDL;
	if (current_read_buffer_ != -1 && current_data_source_)
	{
		err = !current_data_source_->CheckBuffer(current_read_buffer_, SharedMemoryManager::BufferSemaphoreFlags::Reading);
		if (err)
		{
			TLOG_WARNING("SharedMemoryEventReceiver") << "Buffer was in incorrect state, resetting" << TLOG_ENDL;
			current_data_source_ = nullptr;
			current_read_buffer_ = -1;
			current_header_ = nullptr;
			return nullptr;
		}
	}
	TLOG_TRACE("SharedMemoryEventReceiver") << "Already have buffer, returning stored header" << TLOG_ENDL;
	return current_header_;
}

std::set<artdaq::Fragment::type_t> artdaq::SharedMemoryEventReceiver::GetFragmentTypes(bool& err)
{
	if (current_read_buffer_ == -1 || !current_header_ || !current_data_source_) throw cet::exception("AccessViolation") << "Cannot call GetFragmentTypes when not currently reading a buffer! Call ReadHeader() first!";

	err = !current_data_source_->CheckBuffer(current_read_buffer_, SharedMemoryManager::BufferSemaphoreFlags::Reading);
	if (err) return std::set<Fragment::type_t>();

	current_data_source_->ResetReadPos(current_read_buffer_);
	current_data_source_->IncrementReadPos(current_read_buffer_, sizeof(detail::RawEventHeader));
	auto output = std::set<Fragment::type_t>();

	while (current_data_source_->MoreDataInBuffer(current_read_buffer_))
	{
		err = !current_data_source_->CheckBuffer(current_read_buffer_, SharedMemoryManager::BufferSemaphoreFlags::Reading);
		if (err) return std::set<Fragment::type_t>();
		auto fragHdr = reinterpret_cast<artdaq::detail::RawFragmentHeader*>(current_data_source_->GetReadPos(current_read_buffer_));
		output.insert(fragHdr->type);
        current_data_source_->IncrementReadPos(current_read_buffer_, fragHdr->word_count * sizeof(RawDataType));
	}

	return output;
}


std::unique_ptr<artdaq::Fragments> artdaq::SharedMemoryEventReceiver::GetFragmentsByType(bool& err, Fragment::type_t type)
{
	if (!current_data_source_ || !current_header_ || current_read_buffer_ == -1) throw cet::exception("AccessViolation") << "Cannot call GetFragmentsByType when not currently reading a buffer! Call ReadHeader() first!";
	err = !current_data_source_->CheckBuffer(current_read_buffer_, SharedMemoryManager::BufferSemaphoreFlags::Reading);
	if (err) return nullptr;

	current_data_source_->ResetReadPos(current_read_buffer_);
	current_data_source_->IncrementReadPos(current_read_buffer_, sizeof(detail::RawEventHeader));

	std::unique_ptr<Fragments> output(new Fragments());

	while (current_data_source_->MoreDataInBuffer(current_read_buffer_))
	{
		err = !current_data_source_->CheckBuffer(current_read_buffer_, SharedMemoryManager::BufferSemaphoreFlags::Reading);
		if (err) return nullptr;
		auto fragHdr = reinterpret_cast<artdaq::detail::RawFragmentHeader*>(current_data_source_->GetReadPos(current_read_buffer_));
		if (fragHdr->type == type || type == Fragment::InvalidFragmentType)
		{
			output->emplace_back(fragHdr->word_count - detail::RawFragmentHeader::num_words());
			current_data_source_->Read(current_read_buffer_, output->back().headerAddress(), fragHdr->word_count * sizeof(RawDataType));
			output->back().autoResize();
		}
		else
		{
			current_data_source_->IncrementReadPos(current_read_buffer_, fragHdr->word_count * sizeof(RawDataType));
		}
	}

	return output;
}

std::string artdaq::SharedMemoryEventReceiver::printBuffers_(SharedMemoryManager* data_source)
{
	std::ostringstream ostr;
	for (size_t ii = 0; ii < data_source->size(); ++ii)
	{
		ostr << "Buffer " << std::to_string(ii) << ": " << std::endl;

		data_source->ResetReadPos(ii);
		data_source->IncrementReadPos(ii, sizeof(detail::RawEventHeader));

		while (data_source->MoreDataInBuffer(ii))
		{
			auto fragHdr = reinterpret_cast<artdaq::detail::RawFragmentHeader*>(data_source->GetReadPos(ii));
			ostr << "    Fragment " << std::to_string(fragHdr->fragment_id) << ": Sequence ID: " << std::to_string(fragHdr->sequence_id) << ", Type:" << std::to_string(fragHdr->type);
			if (fragHdr->MakeVerboseSystemTypeMap().count(fragHdr->type))
			{
				ostr << " (" << fragHdr->MakeVerboseSystemTypeMap()[fragHdr->type] << ")";
			}
			ostr << ", Size: " << std::to_string(fragHdr->word_count) << " words." << std::endl;
			data_source->IncrementReadPos(ii, fragHdr->word_count * sizeof(RawDataType));
		}

	}
	return ostr.str();
}

std::string artdaq::SharedMemoryEventReceiver::toString()
{
	std::ostringstream ostr;
	ostr << data_.toString() << std::endl;

	ostr << "Data Buffer Fragment Counts: " << std::endl;
	ostr << printBuffers_(&data_);

	if (data_.GetKey() != broadcasts_.GetKey())
	{
		ostr << "Broadcast Buffer Fragment Counts: " << std::endl;
		ostr << printBuffers_(&broadcasts_);
	}

	return ostr.str();
}

void artdaq::SharedMemoryEventReceiver::ReleaseBuffer()
{
	TLOG_TRACE("SharedMemoryEventReceiver") << "ReleaseBuffer BEGIN" << TLOG_ENDL;
	try
	{
		current_data_source_->MarkBufferEmpty(current_read_buffer_);
	}
	catch (cet::exception e)
	{
		TLOG_WARNING("SharedMemoryEventReceiver") << "A cet::exception occured while trying to release the buffer: " << e << TLOG_ENDL;
	}
	catch (...)
	{
		TLOG_ERROR("SharedMemoryEventReceiver") << "An unknown exception occured while trying to release the buffer" << TLOG_ENDL;
	}
	current_read_buffer_ = -1;
	current_header_ = nullptr;
	current_data_source_ = nullptr;
	TLOG_TRACE("SharedMemoryEventReceiver") << "ReleaseBuffer END" << TLOG_ENDL;
}
