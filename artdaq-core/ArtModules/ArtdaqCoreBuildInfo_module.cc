#include "artdaq-core/ArtModules/BuildInfo_module.hh"

#include "artdaq-core/BuildInfo/GetPackageBuildInfo.hh"

#include <string>

namespace artdaq {

  static std::string instanceName = "ArtdaqCoreBuildInfo";
  typedef artdaq::BuildInfo< &instanceName, artdaqcore::GetPackageBuildInfo> ArtdaqCoreBuildInfo;

  DEFINE_ART_MODULE(ArtdaqCoreBuildInfo)
}
