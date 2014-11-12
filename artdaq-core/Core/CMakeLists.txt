# ======================================================================
#
# Build/install script
#
# ======================================================================

# ----------------------------------------------------------------------
# Build this project's library:

art_make( BASENAME_ONLY
  LIBRARY_NAME artdaq-core_Core
  LIB_LIBRARIES
  artdaq-core_Data
  ${FHICLCPP}
  ${Boost_THREAD_LIBRARY}
  ${Boost_SYSTEM_LIBRARY} 
  )

install_headers()
install_source()
