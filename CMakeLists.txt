# This source file is used to build sxbp, a command-line program which
# generates experimental 2D spiral-like shapes based on input binary data.
#
#
#
# Copyright (C) 2016, Joshua Saxby joshua.a.saxby+TNOPLuc8vM==@gmail.com
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU Affero General Public License (version 3),
# as published by the Free Software Foundation.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Affero General Public License for more details.
#
# You should have received a copy of the GNU Affero General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

# begin basic metadata
cmake_minimum_required(VERSION 3.0)

project(sxbp VERSION 0.9.1 LANGUAGES C)
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(
    SXBP_VERSION_STRING
    "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}"
)
set(SXBP_ESCAPED_VERSION_STRING "\"${SXBP_VERSION_STRING}\"")
# end basic metadata

# pass in version of library as preprocessor definitions
add_definitions(-DSXBP_VERSION_MAJOR=${PROJECT_VERSION_MAJOR})
add_definitions(-DSXBP_VERSION_MINOR=${PROJECT_VERSION_MINOR})
add_definitions(-DSXBP_VERSION_PATCH=${PROJECT_VERSION_PATCH})
add_definitions(-DSXBP_VERSION_STRING=${SXBP_ESCAPED_VERSION_STRING})

# used for enabling additional compiler options if supported
include(CheckCCompilerFlag)

function(enable_c_compiler_flag_if_supported flag)
    string(FIND "${CMAKE_C_FLAGS}" "${flag}" flag_already_set)
    if(flag_already_set EQUAL -1)
        check_c_compiler_flag("${flag}" flag_supported)
        if(flag_supported)
            set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${flag}" PARENT_SCOPE)
        endif()
    endif()
endfunction()

# enable extra flags (warnings) if we're not in release mode
if(CMAKE_BUILD_TYPE STREQUAL "Debug" OR CMAKE_BUILD_TYPE STREQUAL "")
    message("Warnings Enabled")
    # enable all warnings about 'questionable constructs'
    enable_c_compiler_flag_if_supported("-Wall")
    # issue 'pedantic' warnings for strict ISO compliance
    enable_c_compiler_flag_if_supported("-pedantic")
    # enable 'extra' strict warnings
    enable_c_compiler_flag_if_supported("-Wextra")
    # convert all warnings into errors
    enable_c_compiler_flag_if_supported("-Werror")
endif()

# dependencies
# add custom dependencies directory
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")
# libsaxbospiral
find_package(SXBP 0.24 EXACT REQUIRED)
include_directories(${SXBP_INCLUDE_DIR})
# Argtable
find_package(Argtable REQUIRED)
include_directories(${ARGTABLE_INCLUDE_DIR})

add_executable(sxbp sxbp.c)

target_link_libraries(sxbp ${SXBP_LIBRARY} ${ARGTABLE_LIBRARY})

install(PROGRAMS sxbp DESTINATION bin)

enable_testing()
# fetch a shell script runner
find_program(COMMAND_INTERPRETER bash)
# only run functional test if we found bash
if(COMMAND_INTERPRETER)
    add_test(
        NAME func_test COMMAND ${COMMAND_INTERPRETER}
        # each script needs to know the path to the sxp cli executable
        "func_test.sh" sxbp "SXBP by saxbophone"
    )
    add_custom_target(
        build_logo ${COMMAND_INTERPRETER}
        "build_logo.sh" sxbp "sxbp.pbm" "SXBP by saxbophone"
    )
else()
    # warn about skipping of functional test script
    message(WARNING "Skipping functional test script, couldn't find Bash Shell")
endif()
