# ======================================================================
#
# Build/install script
#
# ======================================================================

# ----------------------------------------------------------------------
# Build this project's library:

cet_make(
  LIBRARIES
	Boost::thread
	TRACE::TRACE
	cetlib_except::cetlib_except
	messagefacility::MF_MessageLogger
    artdaq_core::artdaq-core_Utilities
	artdaq_core::artdaq-core_Data
  )

install_headers()
install_source()
