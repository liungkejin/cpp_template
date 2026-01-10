#----------------------------------------------------------------
# Generated CMake target import file for configuration "Debug".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "@EXPORT_LIB_NAME@" for configuration "Debug"
set_property(TARGET @EXPORT_LIB_NAME@ APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(@EXPORT_LIB_NAME@ PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_DEBUG "C;CXX"
  IMPORTED_LOCATION_DEBUG "${_IMPORT_PREFIX}/libs/@EXPORT_ARCH@/@EXPORT_LIB_FILE_NAME@"
  )

list(APPEND _cmake_import_check_targets @EXPORT_LIB_NAME@ )
list(APPEND _cmake_import_check_files_for_@EXPORT_LIB_NAME@ "${_IMPORT_PREFIX}/libs/@EXPORT_ARCH@/@EXPORT_LIB_FILE_NAME@" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
