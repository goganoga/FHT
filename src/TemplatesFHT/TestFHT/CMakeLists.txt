cmake_minimum_required(VERSION 2.8)
project(TestFHT CXX)
include(ExternalProject)

if(NOT MSVC)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -pthread")
  set (EXECUTABLE_OUTPUT_PATH bin)
endif()
if(MSVC)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++17")
endif()
find_package(Git REQUIRED)

#FHT
ExternalProject_Add(
    fhtlib
    BINARY_DIR fhtlib/build
    PREFIX fhtlib
    GIT_REPOSITORY https://github.com/goganoga/FHT.git
    TIMEOUT 10
    UPDATE_COMMAND ${GIT_EXECUTABLE} pull
    INSTALL_COMMAND ""
    LOG_DOWNLOAD ON
)
ExternalProject_Get_property(fhtlib BINARY_DIR SOURCE_DIR)
set(FHT_LIBRARIES 
  event
  fht)
set(LINKER 
  ${libevent_BINARY_DIR}/lib
  ${BINARY_DIR}/${CMAKE_BUILD_TYPE}
  ${BINARY_DIR}/libevent/lib/${CMAKE_BUILD_TYPE})
set(HEADER 
  ${SOURCE_DIR}/src/FHT/Interfice)
include_directories(${HEADER})
link_directories(${LINKER})
#build
add_executable(TestFHT "src/TestFHT.cc")
add_dependencies(TestFHT fhtlib)
target_link_libraries(TestFHT ${FHT_LIBRARIES})
