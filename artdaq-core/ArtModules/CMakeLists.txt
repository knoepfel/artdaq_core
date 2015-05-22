
simple_plugin(ArtdaqCoreBuildInfo "module"
  artdaq-core_BuildInfo
  MF_MessageLogger
  fhiclcpp
  cetlib
  )

simple_plugin(PrintBuildInfo "module"
  MF_MessageLogger
  fhiclcpp
  cetlib
  )

install_headers()
install_source()
