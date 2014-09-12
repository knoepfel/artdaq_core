#include "artdaq-core/BuildInfo/GetPackageInfo.hh"

#include "artdaq-core/ArtModules/BuildInfo_module.hh"

#include <string>

namespace artdaq {

  static std::string instanceName = "ArtdaqCoreBuildInfo";
  typedef artdaq::BuildInfo< &instanceName, artdaqcore::PackageInfo> ArtdaqCoreBuildInfo;

  DEFINE_ART_MODULE(ArtdaqCoreBuildInfo)
}
