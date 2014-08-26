art_make(
  LIB_LIBRARIES
  ${ART_PERSISTENCY_COMMON}
  ${ART_UTILITIES}
  ${CETLIB}
  )

install_headers(SUBDIRS detail)
install_source(SUBDIRS detail)
