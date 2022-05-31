cet_register_export_set(SET_NAME PluginTypes NAMESPACE artdaq_plugin_types)

cet_make_library(LIBRARY_NAME fragmentNameHelper INTERFACE
				 EXPORT_SET PluginTypes
				 SOURCE FragmentNameHelper.hh
				 LIBRARIES INTERFACE
				 art_plugin_support::plugin_config_macros
				 art_plugin_support::support_macros
				 artdaq_core::artdaq-core_Data)

cet_make_library(LIBRARY_NAME generator INTERFACE
				 EXPORT_SET PluginTypes
				 SOURCE FragmentGenerator.hh
				 LIBRARIES INTERFACE
				 art_plugin_support::plugin_config_macros
				 art_plugin_support::support_macros
				 artdaq_core::artdaq-core_Data
				 TRACE::TRACE)


include(BasicPlugin)
foreach(type IN ITEMS fragmentNameHelper generator)
  cet_make_plugin_builder(${type} ${type} Modules
    LIBRARIES REG artdaq_plugin_types::${type})
endforeach()

include(fragmentNameHelper)
cet_build_plugin(Artdaq fragmentNameHelper)

cet_make(
  EXCLUDE Artdaq_fragmentNameHelper.cc
  LIBRARIES
  fhiclcpp::fhiclcpp
  TRACE::TRACE
  artdaq_core::artdaq-core_Data
  )


install_headers()
install_source()
