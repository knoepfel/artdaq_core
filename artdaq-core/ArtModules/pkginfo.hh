#ifndef artdaq_core_ArtModules_pkginfo_hh
#define artdaq_core_ArtModules_pkginfo_hh

// JCF, 9/11/14

// Will probably replace this by augmenting the PackageBuildInfo
// struct with the name of the package

namespace artdaq {

    struct pkginfo {

      std::string packageName_;
      std::string packageVersion_;
      std::string buildTimestamp_;
    };

}

#endif
