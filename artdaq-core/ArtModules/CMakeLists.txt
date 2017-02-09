
simple_plugin(ArtdaqCoreBuildInfo "module"
  artdaq-core_BuildInfo
  MF_MessageLogger
  fhiclcpp
  ${CETLIB_LIBS}
  )

simple_plugin(PrintBuildInfo "module"
  MF_MessageLogger
  fhiclcpp
  ${CETLIB_LIBS}
  )

install_headers()
install_source()
