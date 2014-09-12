#ifndef artdaq_core_BuildInfo_GetPackageInfo_hh
#define artdaq_core_BuildInfo_GetPackageInfo_hh

#include "artdaq-core/Data/PackageBuildInfo.hh"

#include <string>

namespace artdaqcore {

  struct PackageInfo {

    static artdaq::PackageBuildInfo getPackageBuildInfo();
  };

}

#endif /* artdaq_core_BuildInfo_GetPackageInfo_hh */
