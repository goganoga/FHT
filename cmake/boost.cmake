`include(ExternalProject)
add_definitions(-D_WIN32_WINNT=0x0601)
add_definitions(-DBOOST_UUID_FORCE_AUTO_LINK)
add_definitions(-DBOOST_ALL_NO_LIB)
add_definitions(-DBOOST_BIND_GLOBAL_PLACEHOLDERS)

set(BOOST_LIBRARIES
  system
  regex
  chrono
  serialization
  thread
)
if(WITH_COROUTINE)
set(BOOST_LIBRARIES
  coroutine
  context
  program_options
  ${BOOST_LIBRARIES}
)
endif()
list(JOIN BOOST_LIBRARIES "|" BOOST_LIBRARIES_STR )
list(APPEND CMAKE_ARGS 
      -DBoost_USE_STATIC_LIBS=ON
      -DBoost_USE_MULTITHREADED=ON
      -DBoost_USE_STATIC_RUNTIME=ON
      -DBOOST_DISABLE_TESTS=ON
      -DCMAKE_CXX_FLAGS=${CMAKE_CXX_FLAGS}
      -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
      -DBOOST_URL=https://boostorg.jfrog.io/artifactory/main/release/1.75.0/source/boost_1_75_0.tar.gz
      -DBOOST_URL_SHA256=aeb26f80e80945e82ee93e5939baebdca47b9dee80a07d3144be1e1a6a66dd6a
      --parallel 16
      -DBOOST_LIBS_OPTIONAL:STRING=${BOOST_LIBRARIES_STR})
ExternalProject_Add(
    Boost
    PREFIX boost-cmake
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
link_directories(${BINARY_DIR})
set (BOOST_LIBS_LIST "")
foreach (boost_lib ${BOOST_LIBRARIES})
    list (APPEND BOOST_LIBS_LIST boost_${boost_lib})
endforeach()
unset (BOOST_LIBRARIES)
set (BOOST_LIBRARIES ${BOOST_LIBS_LIST})
unset (BOOST_LIBS_LIST)

include_directories(${BOOST_INCLUDE_DIR})
