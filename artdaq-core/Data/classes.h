#include "artdaq-core/Data/Fragment.hh"
#include "artdaq-core/Core/QuickVec.hh"
#include "artdaq-core/Data/detail/ParentageMap.hh"
#include "canvas/Persistency/Common/Wrapper.h"
#include "artdaq-core/Data/PackageBuildInfo.hh"
#include <vector>

template class std::vector<artdaq::Fragment>;
template class artdaq::QuickVec<artdaq::RawDataType>;
template class art::Wrapper<std::vector<artdaq::Fragment>>;
template class art::Wrapper<std::vector<artdaq::PackageBuildInfo>>;
template class art::Wrapper<artdaq::PackageBuildInfo>;
