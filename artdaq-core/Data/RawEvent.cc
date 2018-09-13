#include <ostream>
#include "artdaq-core/Data/RawEvent.hh"

namespace artdaq
{
	constexpr uint8_t detail::RawEventHeader::CURRENT_VERSION;
	void RawEvent::print(std::ostream& os) const
	{
		os << "Run " << runID()
			<< ", Subrun " << subrunID()
			<< ", Event " << eventID()
			<< ", SeqID " << sequenceID()
			<< ", FragCount " << numFragments()
			<< ", WordCount " << wordCount()
			<< ", Complete? " << isComplete()
			<< '\n';
		for (auto const& frag : fragments_)
		{
			os << *frag << '\n';
		}
	}
}
