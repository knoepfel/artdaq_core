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
  fhiclcpp
  pthread
  ${MF_MESSAGELOGGER}
  ${Boost_THREAD_LIBRARY}
  ${Boost_SYSTEM_LIBRARY} 
  )

install_headers()
install_source()
