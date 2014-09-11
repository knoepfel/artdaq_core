#ifndef artdaq_core_BuildInfo_GetPackageInfo_h
#define artdaq_core_BuildInfo_GetPackageInfo_h

#include <string>

namespace artdaqcore {

  struct PackageInfo {

    static std::string getPackageName();
    static std::string getPackageVersion(); 
    static std::string getBuildTimestamp();
  };

}

#endif /* artdaq_core_BuildInfo_GetPackageInfo_h */
