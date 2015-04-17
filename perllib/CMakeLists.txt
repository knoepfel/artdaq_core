configure_file($ENV{CETLIB_DIR}/perllib/PluginVersionInfo.pm.in
  ${CMAKE_CURRENT_BINARY_DIR}/CetSkel/${product}/PluginVersionInfo.pm
  @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/CetSkel/${product}/PluginVersionInfo.pm
  DESTINATION ${product}/${version}/perllib/CetSkel/${product}/)

add_subdirectory(CetSkelPlugins)
