
if( DEFINED ENV{CANVAS_DIR} )
set( generator_lib_list art_Persistency_Common 
                	canvas_Persistency_Common
                	art_Utilities 
			canvas_Utilities
			fhiclcpp
			cetlib
			MF_MessageLogger
			MF_Utilities
   )
else()
set( generator_lib_list art_Persistency_Common 
                	art_Utilities 
			fhiclcpp
			cetlib
			MF_MessageLogger
			MF_Utilities
   )
endif()

art_make(
  LIB_LIBRARIES
  artdaq-core_Data
  ${generator_lib_list}
  )

install_headers()
install_source()
