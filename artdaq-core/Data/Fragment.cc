#include "artdaq-core/Data/Fragment.hh"

#include <cmath>
#include <iostream>

using artdaq::detail::RawFragmentHeader;

bool artdaq::fragmentSequenceIDCompare(Fragment i, Fragment j)
{
  return i.sequenceID() < j.sequenceID();
}

artdaq::Fragment::Fragment() :
  vals_(RawFragmentHeader::num_words(), 0)
{
  updateFragmentHeaderWC_();
  fragmentHeader()->metadata_word_count = 0;
}

artdaq::Fragment::Fragment(std::size_t n) :
  vals_(n + RawFragmentHeader::num_words())
{
  // vals ctor w/o init val is used; make sure header is ALL initialized.
  for (iterator ii=vals_.begin();
       ii!=(vals_.begin()+RawFragmentHeader::num_words()); ++ii) *ii=0;
  updateFragmentHeaderWC_();
  fragmentHeader()->type        = Fragment::InvalidFragmentType;
  fragmentHeader()->sequence_id = Fragment::InvalidSequenceID;
  fragmentHeader()->fragment_id = Fragment::InvalidFragmentID;
  fragmentHeader()->timestamp = Fragment::InvalidTimestamp;
  fragmentHeader()->metadata_word_count = 0;
}

artdaq::Fragment::Fragment(sequence_id_t sequenceID,
                           fragment_id_t fragID,
                           type_t type,
						   timestamp_t timestamp) :
  vals_(RawFragmentHeader::num_words(), 0)
{
  updateFragmentHeaderWC_();
  if (type == Fragment::DataFragmentType) {
    // this value is special because it is the default specified
    // in the constructor declaration
    fragmentHeader()->setSystemType(type);
  } else {
    fragmentHeader()->setUserType(type);
  }
  fragmentHeader()->sequence_id = sequenceID;
  fragmentHeader()->fragment_id = fragID;
  fragmentHeader()->timestamp = timestamp;
  fragmentHeader()->metadata_word_count = 0;
}

#if HIDE_FROM_ROOT
void
artdaq::Fragment::print(std::ostream & os) const
{
  os << " Fragment " << fragmentID()
     << ", WordCount " << size()
     << ", Event " << sequenceID()
     << '\n';
}

std::unique_ptr<artdaq::Fragment>
artdaq::Fragment::eodFrag(size_t nFragsToExpect)
{
  std::unique_ptr<artdaq::Fragment> result(new Fragment(static_cast<size_t>(ceil(sizeof(nFragsToExpect) /
                     static_cast<double>(sizeof(value_type))))));
  result->setSystemType(Fragment::EndOfDataFragmentType);
  *result->dataBegin() = nFragsToExpect;
  return result;
}

artdaq::Fragment
artdaq::Fragment::
dataFrag(sequence_id_t sequenceID,
         fragment_id_t fragID,
         RawDataType const * dataPtr,
         size_t dataSize,
		 timestamp_t timestamp)
{
  Fragment result(sequenceID, fragID, timestamp);
  result.resize(dataSize);
  memcpy(result.dataAddress(), dataPtr, (dataSize * sizeof(RawDataType)));
  return result;
}
#endif
