# CMake find script for libsxbp https://github.com/saxbophone/libsxbp
# Based on https://raw.githubusercontent.com/cinemast/libjson-rpc-cpp/master/cmake/FindArgtable.cmake
# - Try to find SXBP
# Once done this will define
#
#  SXBP_FOUND - system has SXBP
#  SXBP_INCLUDE_DIRS - the SXBP include directory
#  SXBP_LIBRARIES - Link these to use SXBP

# TODO: Get it to find the actual correct version of libsxbp

# NOTE: This should use the version of libsxbp requested but right now I don't have the energy to work out how to do it
find_path(
    SXBP_INCLUDE_DIR
    NAMES "sxbp-0/saxbospiral.h"
    DOC "sxbp include dir"
)

find_library(
    SXBP_LIBRARY
    NAMES sxbp
    DOC "sxbp library"
)

set(SXBP_INCLUDE_DIRS ${SXBP_INCLUDE_DIR})
set(SXBP_LIBRARIES ${SXBP_LIBRARY})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
    sxbp DEFAULT_MSG SXBP_INCLUDE_DIR SXBP_LIBRARY
)
mark_as_advanced(SXBP_INCLUDE_DIR SXBP_LIBRARY)
