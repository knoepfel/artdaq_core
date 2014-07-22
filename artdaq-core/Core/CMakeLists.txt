# ======================================================================
#
# Build/install script
#
# ======================================================================

# ----------------------------------------------------------------------
# Build this project's library:

art_make(
  LIB_LIBRARIES
  artdaq-core_Data
  ${FHICLCPP}
  ${Boost_THREAD_LIBRARY}
  ${Boost_SYSTEM_LIBRARY} 
  )

install_headers()
install_source()
