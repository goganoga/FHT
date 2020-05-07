cmake_minimum_required(VERSION 2.8)
project(TestServiceFHT CXX)
include(ExternalProject)

if(NOT MSVC)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -pthread")
  set(pthread event_pthreads)
  set(EXECUTABLE_OUTPUT_PATH bin)
endif()
if(MSVC)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++17")
  set(BUILD_TYPE ${CMAKE_BUILD_TYPE})
  set(LIB_FOR_WIN 
    iphlpapi
    shell32
    advapi32
    ws2_32
    Crypt32)
endif()
add_definitions(-DBOOST_UUID_FORCE_AUTO_LINK)
#ssl
find_package(OpenSSL REQUIRED)
find_package(Git REQUIRED)
add_definitions(-DBOOST_ALL_NO_LIB)
#JSONcpp
list(APPEND CMAKE_ARGS_JSON 
      -DJSONCPP_WITH_TESTS=OFF
      -DJSONCPP_WITH_POST_BUILD_UNITTEST=OFF
      -j 24)
ExternalProject_Add(
    Jsoncpp
    PREFIX jsoncpp
    GIT_REPOSITORY https://github.com/open-source-parsers/jsoncpp.git
    TIMEOUT 10
    UPDATE_COMMAND ${GIT_EXECUTABLE} pull
    CMAKE_ARGS ${CMAKE_ARGS_JSON}
    INSTALL_COMMAND ""
    LOG_DOWNLOAD ON
)

ExternalProject_Get_property(Jsoncpp SOURCE_DIR BINARY_DIR)

set(JSONCPP_INCLUDE_DIR ${SOURCE_DIR}/include)
set(JSONCPP_LIBRARIES_DIR ${BINARY_DIR}/lib/${BUILD_TYPE})
set(JSONCPP_LIBRARIES jsoncpp)
link_directories(${JSONCPP_LIBRARIES_DIR})
include_directories(
  ${JSONCPP_INCLUDE_DIR}
)
#boost
set(BOOST_LIBRARIES
  system
  serialization
  program_options
)
list(APPEND CMAKE_ARGS 
      -DBoost_USE_STATIC_LIBS=ON
      -DBoost_USE_MULTITHREADED=ON
      -DBoost_USE_STATIC_RUNTIME=ON
      -DBOOST_UUID_FORCE_AUTO_LINK=ON
      -DBOOST_DISABLE_TESTS=ON
      -DBOOST_ALL_NO_LIB=ON
      --parallel 16
      -DBOOST_LIBS_OPTIONAL:STRING=system|serialization|program_options)
ExternalProject_Add(
    Boost
    PREFIX boost-cmake_
    GIT_REPOSITORY https://github.com/Orphis/boost-cmake.git
    TIMEOUT 10
    UPDATE_COMMAND ${GIT_EXECUTABLE} pull
    LIST_SEPARATOR |
    CMAKE_ARGS ${CMAKE_ARGS}
    INSTALL_COMMAND ""
    LOG_DOWNLOAD ON
)

ExternalProject_Get_property(Boost BINARY_DIR)
set(BOOST_INCLUDE_DIR ${BINARY_DIR}/_deps/boost-src)
link_directories(${BINARY_DIR}/${BUILD_TYPE})
set (BOOST_LIBS_LIST "")
foreach (boost_lib ${BOOST_LIBRARIES})
    list (APPEND BOOST_LIBS_LIST boost_${boost_lib})
endforeach()
unset (BOOST_LIBRARIES)
set (BOOST_LIBRARIES ${BOOST_LIBS_LIST})
unset (BOOST_LIBS_LIST)

#FHT
ExternalProject_Add(
    fhtlib
    BINARY_DIR fhtlib/build
    PREFIX fhtlib
    GIT_REPOSITORY https://github.com/goganoga/FHT.git
    TIMEOUT 10
    UPDATE_COMMAND ${GIT_EXECUTABLE} pull
    CMAKE_ARGS -DBLOCKING_IO_MODE=ON
    INSTALL_COMMAND ""
    LOG_DOWNLOAD ON
)
ExternalProject_Get_property(fhtlib BINARY_DIR SOURCE_DIR)
set(FHT_LIBRARIES 
    fht
    event
    event_core
    event_extra
    event_openssl
    ${OPENSSL_LIBRARIES}
    ${pthread})
set(LINKER
  ${BINARY_DIR}/${BUILD_TYPE}
  ${BINARY_DIR}/libevent/lib/${BUILD_TYPE})
set(HEADER 
  ${SOURCE_DIR}/src/FHT/Interface
  ${BOOST_INCLUDE_DIR}
  ${OPENSSL_INCLUDE_DIR})
include_directories(${HEADER})
link_directories(${LINKER})
#build
add_executable(TestServiceFHT "src/TestServiceFHT.cc" "src/Test/Test.cpp")
add_dependencies(TestServiceFHT fhtlib)
target_link_libraries(TestServiceFHT 
  ${LIB_FOR_WIN}
  ${FHT_LIBRARIES}
  ${BOOST_LIBRARIES}
  ${JSONCPP_LIBRARIES})

add_custom_command(
        TARGET TestServiceFHT POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
                ${CMAKE_CURRENT_SOURCE_DIR}/src/config.json
                ${CMAKE_CURRENT_BINARY_DIR}/${EXECUTABLE_OUTPUT_PATH}/config.json)
