# ======================================================================
#  artdaq-core main build file
#
#  cd .../path/to/build/directory
#  source .../path/to/artdaq-core/ups/setup_for_development
#  cmake [-DCMAKE_INSTALL_PREFIX=/install/path]
#        -Dqualifier:STRING=$CETPKG_QUAL
#        -DCMAKE_BUILD_TYPE=$CETPKG_TYPE
#        $CETPKG_SOURCE
#  make
#  make test
#  make install
#  make package
# ======================================================================

# use cmake 2.8 or later
cmake_minimum_required (VERSION 2.8)

# cetbuildtools contains our cmake modules
SET ( CETBUILDTOOLS_VERSION $ENV{CETBUILDTOOLS_VERSION} )
IF (NOT CETBUILDTOOLS_VERSION)
    MESSAGE (FATAL_ERROR "ERROR: setup cetbuildtools to get the cmake modules")
ENDIF()

set(CMAKE_MODULE_PATH $ENV{ART_DIR}/Modules
		      $ENV{CETBUILDTOOLS_DIR}/Modules
		      ${CMAKE_MODULE_PATH})

include(CetCMakeEnv)
cet_cmake_env()

# -D_GLIBCXX_USE_NANOSLEEP is needed to make std::this_thread::sleep_for available.
cet_set_compiler_flags(DIAGS VIGILANT
  WERROR
  NO_UNDEFINED
  EXTRA_FLAGS -pedantic
  EXTRA_CXX_FLAGS -std=c++11  -D_GLIBCXX_USE_NANOSLEEP
)

cet_report_compiler_flags()

# these are minimum required versions, not the actual product versions
find_ups_product( art v1_08_10 )
find_ups_product( cetbuildtools v3_07_03 )

cet_check_gcc()

# include search path
# notice use of ENV to invoke an external variable
include_directories ( $ENV{LIBSIGCPP_INC}/sigc++-2.0 )
include_directories ( $ENV{LIBSIGCPP_LIB}/sigc++-2.0/include )
include_directories ( $ENV{SMC_HOME}/lib/C++ )

#  Find all the libraries needed by our dependent CMakeList.txt files
find_library( CPPUNIT NAMES cppunit   PATHS $ENV{CPPUNIT_LIB}   NO_DEFAULT_PATH )
find_library( SIGC    NAMES sigc-2.0  PATHS $ENV{LIBSIGCPP_LIB} NO_DEFAULT_PATH )

# Debug streamer.
string(TOUPPER ${CMAKE_BUILD_TYPE} BTYPE_UC)
if(BTYPE_UC STREQUAL DEBUG)
  add_definitions(-DDEBUGME)
endif()

# macros for art_dictionary and simple_plugin
include(ArtDictionary)
include(ArtMake)
include(BuildPlugins)

# testing
add_subdirectory(test)

# source
add_subdirectory(artdaq-core)

# ups - table and config files
add_subdirectory(ups)

# packaging utility
include(UseCPack)
