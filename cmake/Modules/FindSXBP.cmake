# CMake find script for libsxbp https://github.com/saxbophone/libsxbp
# Based on https://raw.githubusercontent.com/cinemast/libjson-rpc-cpp/master/cmake/FindArgtable.cmake
#
# - Try to find SXBP
# Once done this will define
#
#  SXBP_FOUND - system has SXBP
#  SXBP_INCLUDE_DIRS - the SXBP include directory
#  SXBP_LIBRARIES - Link these to use SXBP
# 
# This script will check for which versions of libsxbp are available and try to provide the 

message(${SXBP_FIND_VERSION})

find_path(
    SXBP_INCLUDE_DIR
    NAMES "sxbp-0/saxbospiral.h"
    DOC "sxbp include dir"
)

# list(APPEND CMAKE_FIND_LIBRARY_SUFFIXES .so.0)
# list(APPEND CMAKE_FIND_LIBRARY_SUFFIXES .so.24)
list(APPEND CMAKE_FIND_LIBRARY_SUFFIXES .so.24.2)

find_library(
    SXBP_LIBRARY
    NAMES sxbp
    DOC "sxbp library"
)

set(SXBP_INCLUDE_DIRS ${SXBP_INCLUDE_DIR})
set(SXBP_LIBRARIES ${SXBP_LIBRARY})
set(SXBP_VERSION "0.24.0")


include(FindPackageHandleStandardArgs)


find_package_handle_standard_args(
    sxbp
    REQUIRED_VARS SXBP_LIBRARY SXBP_INCLUDE_DIR
    VERSION_VAR SXBP_VERSION
)

mark_as_advanced(SXBP_INCLUDE_DIR SXBP_LIBRARY)
