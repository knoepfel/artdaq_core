#include "artdaq-core/ArtModules/BuildInfo_module.hh"

#include "artdaq-core/BuildInfo/GetPackageInfo.h"

#include <string>

namespace artdaq {

  static std::string instanceName = "ArtdaqCoreBuildInfo";
  typedef artdaq::BuildInfo< &instanceName, artdaqcore::PackageInfo> ArtdaqCoreBuildInfo;

  DEFINE_ART_MODULE(ArtdaqCoreBuildInfo)
}
