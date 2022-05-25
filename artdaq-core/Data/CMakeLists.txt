cet_make(
NO_DICTIONARY
  LIBRARIES
  TRACE::TRACE
  cetlib_except::cetlib_except
  artdaq_core::artdaq-core_Utilities
  )

  art_dictionary(DICTIONARY_LIBRARIES artdaq-core_Data)
  get_target_property(DictCompileOptions artdaq-core_Data_dict COMPILE_OPTIONS)
  if(DictCompileOptions)
	string(REPLACE "-fsanitize=thread" "" DictCompileOptions "${DictCompileOptions}")
	string(REPLACE "-fsanitize=address" "" DictCompileOptions "${DictCompileOptions}")
	string(REPLACE "-ltsan" "" DictCompileOptions "${DictCompileOptions}")
	string(REPLACE "-lasan" "" DictCompileOptions "${DictCompileOptions}")

	set_target_properties(artdaq-core_Data_dict PROPERTIES COMPILE_OPTIONS "${DictCompileOptions}")
  endif()

install_headers(SUBDIRS detail)
install_source(SUBDIRS detail)
