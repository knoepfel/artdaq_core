
simple_plugin(ArtdaqCoreBuildInfo "module"
  artdaq-core_BuildInfo
  ${MF_MESSAGELOGGER}
  ${FHICLCPP}
  ${CETLIB}
  ${Boost_FILESYSTEM_LIBRARY}
  ${Boost_SYSTEM_LIBRARY}
  )

simple_plugin(PrintBuildInfo "module"
  ${MF_MESSAGELOGGER}
  ${FHICLCPP}
  ${CETLIB}
  ${Boost_FILESYSTEM_LIBRARY}
  ${Boost_SYSTEM_LIBRARY}
  )

install_headers()
install_source()
