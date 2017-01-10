
art_make(
  LIB_LIBRARIES 
  art_Persistency_Common 
  canvas_Persistency_Common
  art_Utilities 
  canvas_Utilities
  cetlib )

install_headers(SUBDIRS detail)
install_source(SUBDIRS detail)
