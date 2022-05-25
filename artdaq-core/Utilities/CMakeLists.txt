
cet_make(
	EXCLUDE GenFile_mfPlugin.cc ExceptionStackTrace.cc
  LIBRARIES
  ${EXCEPTIONSTACKTRACE_LIB}
	fhiclcpp::fhiclcpp
	Boost::date_time
	cetlib::cetlib
	canvas::canvas
	TRACE::TRACE
	messagefacility::MF_MessageLogger
)

cet_make_library(
	LIBRARY_NAME  artdaq-core_Utilities_ExceptionStackTrace
	SOURCE 
	ExceptionStackTrace.cc 
  LIBRARIES
	-ldl
	TRACE::TRACE
)

include(mfPlugin)
mfPlugin(GenFile LIBRARIES REG TRACE::TRACE messagefacility::MF_MessageLogger)


install_headers()
install_source()
