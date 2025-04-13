#
# CoPilot generated the first draft of this file.
#

# Set minimum CMake version
cmake_minimum_required(VERSION 3.10)

# Project name
project(code_test)

# Set the C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Enable debug information
set(CMAKE_BUILD_TYPE Debug)

# Ensure debug symbols are included
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g")

# Ensure the compiler generates debug information with source code paths
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fdebug-prefix-map=${CMAKE_SOURCE_DIR}=.")

# Process the directories
add_subdirectory(${CMAKE_SOURCE_DIR}/src)
