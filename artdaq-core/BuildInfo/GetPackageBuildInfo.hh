#ifndef artdaq_core_BuildInfo_GetPackageBuildInfo_hh
#define artdaq_core_BuildInfo_GetPackageBuildInfo_hh

#include "artdaq-core/Data/PackageBuildInfo.hh"

namespace artdaqcore {

  struct GetPackageBuildInfo {

    static artdaq::PackageBuildInfo getPackageBuildInfo();
  };

}

#endif /* artdaq_core_BuildInfo_GetPackageBuildInfo_hh */
