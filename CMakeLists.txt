cmake_minimum_required(VERSION 2.8)
project(FHT CXX)
include(ExternalProject)
set (EXECUTABLE_OUTPUT_PATH lib)

if(NOT MSVC)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -pthread")
endif()
if(MSVC)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++17")
  set(LIB_FOR_WIN ws2_32)
endif()
find_package(Git REQUIRED)
set(build ${CMAKE_CURRENT_BINARY_DIR})
#libevent
ExternalProject_Add(
    libevent
    PREFIX libevent
    GIT_REPOSITORY https://github.com/libevent/libevent.git
    TIMEOUT 10
    UPDATE_COMMAND ${GIT_EXECUTABLE} pull
    BINARY_DIR libevent
    CMAKE_ARGS 
      -DEVENT__LIBRARY_TYPE=STATIC
      -DEVENT__BUILD_SHARED_LIBRARIES=OFF
      -DEVENT__DISABLE_TESTS=ON
      -DEVENT__DISABLE_REGRESS=ON
      -DEVENT__DISABLE_SAMPLES=ON
      -DEVENT__DISABLE_BENCHMARK=ON
      --parallel 16
    INSTALL_COMMAND ""
    LOG_DOWNLOAD ON
)
ExternalProject_Get_property(libevent BINARY_DIR SOURCE_DIR)
set(libevent_BINARY_DIR ${BINARY_DIR})
set(libevent_SOURCE_DIR ${SOURCE_DIR})
link_directories(${libevent_BINARY_DIR}/lib)
set(LibEvent_LIBRARIES 
  event)

#FHT
set(SOURCE_LIB 
  src/FHT/Common/Controller/Hendler/Hendler.cpp
  src/FHT/Common/Controller/Task/Task.cpp
  src/FHT/Common/Controller/Server/InitialSer.cpp
  src/FHT/Common/Controller/Server/Server.cpp
  src/FHT/Common/Controller/Client/Client.cpp)
  
set(HEADERS 
  src/FHT/Interface)
set(HEADER 
  src/FHT/Common
  ${libevent_SOURCE_DIR}/include
  ${libevent_BINARY_DIR}/include)
message(${HEADER})
add_library(fht STATIC ${SOURCE_LIB})
add_dependencies(fht 
  libevent)
target_include_directories(fht PUBLIC ${HEADERS})
target_include_directories(fht PRIVATE ${HEADER})
target_link_libraries(fht
  ${LIB_FOR_WIN}
  ${LibEvent_LIBRARIES})
set(FHT_LIBRARIES fht)
