cet_make(
	EXCLUDE GenFile_mfPlugin.cc
  LIBRARIES 
  MF_MessageLogger
  MF_Utilities
  fhiclcpp
  ${CETLIB_LIBS}
  ${Boost_DATE_TIME_LIBRARY}
  ${Boost_FILESYSTEM_LIBRARY}
  ${Boost_SYSTEM_LIBRARY}
)

basic_plugin( "GenFile" "mfPlugin"
    ${MF_MESSAGELOGGER}
  ${MF_UTILITIES}
${MF_MESSAGESERVICE}
    ${FHICLCPP}
    ${CETLIB}
	${CETLIB_EXCEPT}
    ${Boost_SYSTEM_LIBRARY}
    ${Boost_DATE_TIME_LIBRARY}
)

install_headers()
install_source()
