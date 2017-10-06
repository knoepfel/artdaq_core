cet_make(
  LIBRARIES 
  MF_MessageLogger
  MF_Utilities
  fhiclcpp
  ${CETLIB_LIBS}
  ${Boost_DATE_TIME_LIBRARY}
  ${Boost_FILESYSTEM_LIBRARY}
  ${Boost_SYSTEM_LIBRARY}
)

install_headers()
install_source()
