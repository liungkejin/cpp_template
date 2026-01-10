#----------------------------------------------------------------
# Generated CMake target import file for configuration "Release".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "@EXPORT_LIB_NAME@" for configuration "Release"
set_property(TARGET @EXPORT_LIB_NAME@ APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(@EXPORT_LIB_NAME@ PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_RELEASE "C;CXX"
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/libs/@EXPORT_ARCH@/@EXPORT_LIB_FILE_NAME@"
  )

list(APPEND _cmake_import_check_targets @EXPORT_LIB_NAME@ )
list(APPEND _cmake_import_check_files_for_@EXPORT_LIB_NAME@ "${_IMPORT_PREFIX}/libs/@EXPORT_ARCH@/@EXPORT_LIB_FILE_NAME@" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
