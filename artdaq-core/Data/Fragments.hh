#ifndef artdaq_core_Data_Fragments_hh
#define artdaq_core_Data_Fragments_hh

#include <memory>
#include <vector>
#include "artdaq-core/Data/Fragment.hh"

namespace artdaq {
  typedef std::vector<Fragment>     Fragments;
  typedef std::unique_ptr<Fragment> FragmentPtr;
  typedef std::vector<FragmentPtr>  FragmentPtrs;
}

#endif /* artdaq_core_Data_Fragments_hh */
