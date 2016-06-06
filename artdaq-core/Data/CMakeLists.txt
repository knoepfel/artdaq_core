
if( DEFINED ENV{CANVAS_DIR} )
set( data_lib_list art_Persistency_Common 
                   canvas_Persistency_Common
                   art_Utilities 
		   canvas_Utilities
                   cetlib
   )
else()
set( data_lib_list art_Persistency_Common 
                   art_Utilities 
                   cetlib
   )
endif()

art_make(
  LIB_LIBRARIES ${data_lib_list} )

install_headers(SUBDIRS detail)
install_source(SUBDIRS detail)
