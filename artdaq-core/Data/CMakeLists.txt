
art_make(
  LIB_LIBRARIES 
  art_Persistency_Common 
  art_Utilities 
  ${CETLIB_LIBS} )

install_headers(SUBDIRS detail)
install_source(SUBDIRS detail)
