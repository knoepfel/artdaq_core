# ======================================================================
#
# Build/install script
#
# ======================================================================

# ----------------------------------------------------------------------
# Build this project's library:

cet_make(
  LIBRARIES
  artdaq-core_Data
  artdaq-core_Utilities
  fhiclcpp
  pthread
  ${MF_MESSAGELOGGER}
  ${Boost_THREAD_LIBRARY}
  ${Boost_SYSTEM_LIBRARY} 
  )

install_headers()
install_source()
