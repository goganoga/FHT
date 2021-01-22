cmake_minimum_required(VERSION 3.15 FATAL_ERROR)
project(FHT CXX C)
set (EXECUTABLE_OUTPUT_PATH ../build_example)
if(NOT MSVC)
  if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE MINSIZEREL)
  endif()
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -std=c++17 -pthread")
endif()
if(MSVC)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++17 /bigobj")
endif()

find_package(Git REQUIRED)
execute_process(COMMAND ${GIT_EXECUTABLE} submodule update --init --recursive
                        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                        RESULT_VARIABLE GIT_SUBMOD_RESULT)

option(BLOCKING_IO_MODE "Build the blocking io socket" ON)
option(EXAMPLE_BUILD "Build example bin for testing functional" ON)
option(WITH_COROUTINE "Build project with coroutines" ON)
set(DBCOMPILE "None" CACHE STRING "None | Postgresql | sqlite")

include(cmake/boost.cmake)
include(cmake/fht.cmake)
