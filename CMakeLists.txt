#
# CoPilot generated the first draft of this file.
#

# Set minimum CMake version
cmake_minimum_required ( VERSION 3.10 )

# Project name
project ( code_test )

# Set the C++ standard
set ( CMAKE_CXX_STANDARD 20 )
set ( CMAKE_CXX_STANDARD_REQUIRED True )

# Enable debug information
set ( CMAKE_BUILD_TYPE Debug )

# Ensure the compiler generates debug information with source code paths
set ( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fdebug-prefix-map=${CMAKE_SOURCE_DIR}=." )

# Make warning the same as errors.
set ( CMAKE_COMPILE_WARNING_AS_ERROR ON )

# Make the compiler more strict.
if ( CMAKE_CXX_COMPILER_ID STREQUAL "MSVC" )
	add_compile_options ( /Wall )
elseif ( CMAKE_CXX_COMPILER_ID STREQUAL "GNU" )
	add_compile_options ( -Wall )
endif()

# Do not warn about CGAL in debug build.
set ( CGAL_DO_NOT_WARN_ABOUT_CMAKE_BUILD_TYPE True )

# Find these packages.
find_package ( Eigen3 REQUIRED )
find_package ( CGAL REQUIRED )

# Define debug preprocessor symbol when compiling a debug build.
add_compile_definitions ( "$<$<CONFIG:DEBUG>:_DEBUG>" )

# Include directories for Eigen
include_directories ( ${EIGEN3_INCLUDE_DIR} )

# Process the directories
add_subdirectory ( ${CMAKE_SOURCE_DIR}/src )
