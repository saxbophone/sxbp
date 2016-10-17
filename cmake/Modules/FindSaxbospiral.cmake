# CMake find script for libsaxbospiral https://github.com/saxbophone/libsaxbospiral
# Based on https://raw.githubusercontent.com/cinemast/libjson-rpc-cpp/master/cmake/FindArgtable.cmake
# - Try to find SAXBOSPIRAL
# Once done this will define
#
#  SAXBOSPIRAL_FOUND - system has SAXBOSPIRAL
#  SAXBOSPIRAL_INCLUDE_DIRS - the SAXBOSPIRAL include directory
#  SAXBOSPIRAL_LIBRARIES - Link these to use SAXBOSPIRAL

find_path(
    SAXBOSPIRAL_INCLUDE_DIR
    NAMES "saxbospiral/saxbospiral.h"
    DOC "saxbospiral include dir"
)

find_library(
    SAXBOSPIRAL_LIBRARY
    NAMES saxbospiral
    DOC "saxbospiral library"
)

set(SAXBOSPIRAL_INCLUDE_DIRS ${SAXBOSPIRAL_INCLUDE_DIR})
set(SAXBOSPIRAL_LIBRARIES ${SAXBOSPIRAL_LIBRARY})

# handle the QUIETLY and REQUIRED arguments and set JSONCPP_FOUND to TRUE
# if all listed variables are TRUE, hide their existence from configuration view
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
    saxbospiral DEFAULT_MSG SAXBOSPIRAL_INCLUDE_DIR SAXBOSPIRAL_LIBRARY
)
mark_as_advanced(SAXBOSPIRAL_INCLUDE_DIR SAXBOSPIRAL_LIBRARY)
