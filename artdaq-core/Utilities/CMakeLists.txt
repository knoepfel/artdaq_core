
find_library(LIBRT NAMES rt) 
if(LIBRT)
  set(FOUND_RT ${LIBRT})
endif()

cet_make(
	EXCLUDE GenFile_mfPlugin.cc ExceptionStackTrace.cc
  LIBRARIES
  ${MF_MESSAGELOGGER}
  cetlib_except
  cetlib
  fhiclcpp
  ${FOUND_RT}
	${EXCEPTIONSTACKTRACE_LIB}
	Boost::filesystem
)

cet_make_library(
	LIBRARY_NAME  artdaq-core_Utilities_ExceptionStackTrace
	SOURCE 
	ExceptionStackTrace.cc 
  LIBRARIES
	-ldl
)

basic_plugin( "GenFile" "mfPlugin"
  fhiclcpp
  cetlib_except
MF_MessageLogger
)

install_headers()
install_source()
