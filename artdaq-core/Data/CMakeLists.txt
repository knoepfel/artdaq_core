
FILE( GLOB src_files *.c *.cc *.cpp *.C *.cxx )

art_make_library( SOURCE ${src_files}
                  LIBRARIES ${ART_PERSISTENCY_COMMON}
			    ${ART_UTILITIES}
			    ${CETLIB}
                 )

art_dictionary(  )

install_headers(SUBDIRS detail)
install_source(SUBDIRS detail)
