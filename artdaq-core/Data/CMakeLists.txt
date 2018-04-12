cet_make(
NO_DICTIONARY
  LIBRARIES 
  ${CETLIB_LIBS} 
  )

cet_remove_compiler_flags(-fsanitize=address)
  art_dictionary(DICTIONARY_LIBRARIES artdaq-core_Data)

install_headers(SUBDIRS detail)
install_source(SUBDIRS detail)
