#include "artdaq-core/Data/Fragment.hh"
#include "artdaq-core/Data/RonVec.hh"
#include "art/Persistency/Common/Wrapper.h"
#include "artdaq-core/Data/PackageBuildInfo.hh"
#include <vector>

template class std::vector<artdaq::Fragment>;
template class RonVec<unsigned long>;
template class art::Wrapper<std::vector<artdaq::Fragment> >;
template class art::Wrapper<artdaq::PackageBuildInfo>;
