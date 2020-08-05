cmake_minimum_required(VERSION 3.15 FATAL_ERROR)
project(FHT CXX C)
include(ExternalProject)
include(submodule/boost.cmake)
set (EXECUTABLE_OUTPUT_PATH ../build_simple)

if(NOT MSVC)
  if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE MINSIZEREL)
  endif()
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -std=c++17 -pthread")
  set(pthread
    event_pthreads
    pq)
endif()
if(MSVC)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++17")
  set(LIB_FOR_WIN 
    iphlpapi
    shell32
    advapi32
    ws2_32
    Crypt32)
endif()
find_package(Git REQUIRED)
find_package(OpenSSL REQUIRED)

option(BLOCKING_IO_MODE "Build the blocking io socket" OFF)
option(SAMPLE_BUILD "Build sample bin for testing functional" ON)
set(DBCOMPILE "None" CACHE STRING "None | Postgresql")

if(SAMPLE_BUILD)
  set(DBCOMPILE "Postgresql")
endif()
if(BLOCKING_IO_MODE)
  message(STATUS "Build the blocking io socket")
  add_compile_definitions(BLOCKING_IO)
endif()
#submodule sync
execute_process(COMMAND ${GIT_EXECUTABLE} submodule update --init --recursive
                        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                        RESULT_VARIABLE GIT_SUBMOD_RESULT)

message(STATUS "Use database: ${DBCOMPILE}")
if(${DBCOMPILE} STREQUAL "Postgresql")
  find_package(PostgreSQL REQUIRED)
  add_compile_definitions(DBPOSTGRESQL)
  set(DBCPP 
    src/FHT/Common/DBFacade/Postgresql/Postgresql.cpp
    src/FHT/Common/DBFacade/Postgresql/PostgresConnection.cpp
  )
endif()
if(${DBCOMPILE} STREQUAL "None")
  add_compile_definitions(DBNONE)
endif()
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
      -DEVENT__DISABLE_MBEDTLS=ON
      --parallel 16
    INSTALL_COMMAND ""
    LOG_DOWNLOAD ON
)
ExternalProject_Get_property(libevent BINARY_DIR SOURCE_DIR)
set(libevent_BINARY_DIR ${BINARY_DIR})
set(libevent_SOURCE_DIR ${SOURCE_DIR})
set(LibEvent_LIBRARIES 
  event
  event_core
  event_extra
  event_openssl)

#FHT
set(BUILDER_LIB
  src/FHT/lib/base64.cpp
  src/FHT/lib/HostCheckCurl.cpp
)
set(SOURCE_LIB 
  ${BUILDER_LIB}
  src/FHT/Common/Controller/Hendler/Hendler.cpp
  src/FHT/Common/Controller/Logger/Logger.cpp
  src/FHT/Common/Controller/Task/Task.cpp
  src/FHT/Common/Controller/Server/InitialSer.cpp
  src/FHT/Common/Controller/Server/Server.cpp
  src/FHT/Common/Controller/Client/Client.cpp
  src/FHT/Common/Controller/Client/WebClient.cpp
  src/FHT/Common/Controller/Server/WebSocket/User.cpp
  src/FHT/Common/Controller/Server/WebSocket/Frame.cpp
  src/FHT/Common/Controller/Server/WebSocket/WebSocket.cpp
  src/FHT/Common/Controller/Server/WebSocket/Connection.cpp
  src/FHT/Common/DBFacade/DBFacade.cpp
  ${DBCPP})
set(HEADER_LIB
  src/FHT/lib
  src/FHT/Interface
)
set(HEADERS 
  ${HEADER_LIB}
  ${boost_Header}
  ${OPENSSL_INCLUDE_DIR}
  ${PostgreSQL_INCLUDE_DIRS})
set(HEADER 
  src/FHT/Common
  ${libevent_SOURCE_DIR}/include
  ${libevent_BINARY_DIR}/include)
add_library(fht STATIC ${SOURCE_LIB})
add_dependencies(fht 
  libevent
  ${boost_Deps})
target_include_directories(fht PUBLIC ${HEADERS})
target_include_directories(fht PRIVATE ${HEADER})
target_link_directories(fht PUBLIC ${libevent_BINARY_DIR}/lib)
set(FHT_LIBRARIES 
  fht
  ${PostgreSQL_LIBRARIES}
  ${LIB_FOR_WIN}
  ${LibEvent_LIBRARIES}
  ${OPENSSL_LIBRARIES})
#target_link_libraries(fht ${FHT_LIBRARIES})

set(LINKER
  ${BINARY_DIR}/${BUILD_TYPE}
  ${BINARY_DIR}/libevent/lib/${BUILD_TYPE}
  ${PostgreSQL_LIBRARY_DIRS})
link_directories(${LINKER})
if(SAMPLE_BUILD)#JSONcpp
  list(APPEND CMAKE_ARGS_JSON 
        -DJSONCPP_WITH_TESTS=OFF
        -DJSONCPP_WITH_POST_BUILD_UNITTEST=OFF
        -DCMAKE_CXX_FLAGS=${CMAKE_CXX_FLAGS}
        -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
        --parallel 16
       )
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
  set(JSONCPP_LIBRARIES jsoncpp_static)
  link_directories(${JSONCPP_LIBRARIES_DIR})
  include_directories(
    ${JSONCPP_INCLUDE_DIR}
  )

  #TestFHT
  add_executable(TestFHT 
    "src/Sample/TestTaskHendler/TestTaskHendler.cpp")
  add_dependencies(TestFHT fht)
  target_link_libraries(TestFHT 
    fht
    ${FHT_LIBRARIES})

  #TestServiceFHT
  add_executable(TestServiceFHT 
    "src/Sample/TestService/TestService.cpp" 
    "src/Sample/TestService/Test/Test.cpp")
  add_dependencies(TestServiceFHT 
    Jsoncpp
    fht)
  target_link_libraries(TestServiceFHT 
    fht
    ${FHT_LIBRARIES}
    ${JSONCPP_LIBRARIES})
  add_custom_command(
          TARGET TestServiceFHT POST_BUILD
          COMMAND ${CMAKE_COMMAND} -E copy
                  ${CMAKE_CURRENT_SOURCE_DIR}/src/Sample/TestService/config.json
                  ${CMAKE_CURRENT_BINARY_DIR}/${EXECUTABLE_OUTPUT_PATH}/${BUILD_TYPE}/config.json)

  #TestPostgresConnect
  add_executable(TestPostgresConnectFHT 
    "src/Sample/TestPostgresConnect/TestPostgresConnect.cpp" 
    "src/Sample/TestPostgresConnect/Config/Config.cpp")
  add_dependencies(TestPostgresConnectFHT 
    Jsoncpp
    fht)
  target_link_libraries(TestPostgresConnectFHT 
    fht
    ${FHT_LIBRARIES}
    ${JSONCPP_LIBRARIES})
  add_custom_command(
          TARGET TestPostgresConnectFHT POST_BUILD
          COMMAND ${CMAKE_COMMAND} -E copy
                  ${CMAKE_CURRENT_SOURCE_DIR}/src/Sample/TestPostgresConnect/Config/configSQL.json
                  ${CMAKE_CURRENT_BINARY_DIR}/${EXECUTABLE_OUTPUT_PATH}/${BUILD_TYPE}/configSQL.json)

endif()
