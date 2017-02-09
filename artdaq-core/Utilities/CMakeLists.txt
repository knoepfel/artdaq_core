art_make(
  LIB_LIBRARIES 
  art_Utilities 
  MF_MessageLogger
  MF_Utilities
  fhiclcpp
  ${CETLIB_LIBS}
  ${Boost_DATE_TIME_LIBRARY}
)

install_headers()
install_source()
