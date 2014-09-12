
simple_plugin(ArtdaqCoreBuildInfo "module"
  artdaq-core_BuildInfo
  ${MF_MESSAGELOGGER}
  ${FHICLCPP}
  ${CETLIB}
)

simple_plugin(PrintBuildInfo "module"
  ${MF_MESSAGELOGGER}
  ${FHICLCPP}
  ${CETLIB}
)

install_headers()
install_source()
