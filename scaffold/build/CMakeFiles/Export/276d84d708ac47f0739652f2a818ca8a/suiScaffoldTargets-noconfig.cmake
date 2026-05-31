#----------------------------------------------------------------
# Generated CMake target import file.
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "suiScaffold::suiScaffold" for configuration ""
set_property(TARGET suiScaffold::suiScaffold APPEND PROPERTY IMPORTED_CONFIGURATIONS NOCONFIG)
set_target_properties(suiScaffold::suiScaffold PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_NOCONFIG "CXX"
  IMPORTED_LOCATION_NOCONFIG "${_IMPORT_PREFIX}/lib/libsuiScaffold.a"
  )

list(APPEND _cmake_import_check_targets suiScaffold::suiScaffold )
list(APPEND _cmake_import_check_files_for_suiScaffold::suiScaffold "${_IMPORT_PREFIX}/lib/libsuiScaffold.a" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
