#ifndef artdaq_core_Data_PackageBuildInfo_hh
#define artdaq_core_Data_PackageBuildInfo_hh

#include <string>

namespace artdaq {
  class PackageBuildInfo;
}

class artdaq::PackageBuildInfo {
public:
  explicit PackageBuildInfo() {}

  std::string getPackageName() const { return packageName_; }
  std::string getPackageVersion() const { return packageVersion_; }
  std::string getBuildTimestamp() const { return buildTimestamp_; }
  void setPackageName(std::string str) { packageName_ = str; }
  void setPackageVersion(std::string str) { packageVersion_ = str; }
  void setBuildTimestamp(std::string str) { buildTimestamp_ = str; }

private:

  std::string packageName_;
  std::string packageVersion_;
  std::string buildTimestamp_;
};

#endif /* artdaq_core_Data_PackageBuildInfo_hh */
