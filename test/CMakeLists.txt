# This is the project CMakeLists.txt file for the test subproject
cmake_minimum_required(VERSION 3.16)

# Include the components directory of the main application:
#
set(EXTRA_COMPONENT_DIRS "../components")

set(TEST_COMPONENTS "stratum_message" CACHE STRING "List of components to test")
#list(APPEND TEST_COMPONENTS "other_component")

include($ENV{IDF_PATH}/tools/cmake/project.cmake)

# "Trim" the build. Include the minimal set of components, main, and anything it depends on.
project(unit_test_test)

file(TO_NATIVE_PATH "${CMAKE_CURRENT_BINARY_DIR}/coverage_report" _coverage_path)

idf_create_coverage_report(${_coverage_path})
idf_clean_coverage_report(${_coverage_path})