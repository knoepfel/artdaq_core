
if( DEFINED ENV{CANVAS_DIR} )
set( util_lib_list art_Utilities 
		   canvas_Utilities
		   MF_MessageLogger
		   MF_Utilities
		   fhiclcpp
		   cetlib
                   ${Boost_DATE_TIME_LIBRARY}
   )
else()
set( util_lib_list art_Utilities 
		   MF_MessageLogger
		   MF_Utilities
		   fhiclcpp
		   cetlib
                   ${Boost_DATE_TIME_LIBRARY}
   )
endif()
art_make(
  LIB_LIBRARIES ${util_lib_list}
)

install_headers()
install_source()
