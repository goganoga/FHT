find_package(OpenSSL REQUIRED)

if(SAMPLE_BUILD)
  set(DBCOMPILE "Postgresql")
endif()

if(BLOCKING_IO_MODE)
  message(STATUS "Build the blocking io socket")
  add_definitions(-DBLOCKING_IO)
endif()

message(STATUS "Use database: ${DBCOMPILE}")
if(${DBCOMPILE} STREQUAL "Postgresql")
  find_package(PostgreSQL REQUIRED)
  add_definitions(-DDBPOSTGRESQL)
  set(DBCPP 
    src/FHT/Common/DBFacade/Postgresql/Postgresql.cpp
    src/FHT/Common/DBFacade/Postgresql/PostgresConnection.cpp
  )
endif()

if(${DBCOMPILE} STREQUAL "sqlite")
  add_definitions(-DDBSQLITE)
  set(DBCPP 
    src/FHT/Common/DBFacade/SQLite/SQLite.cpp
    src/FHT/Common/DBFacade/Postgresql/sqliteConnection.cpp
  )
endif()

if(${DBCOMPILE} STREQUAL "None")
  add_definitions(-DDBNONE)
endif()

set(SOURCE_LIB 
  src/FHT/Common/Controller/Hendler/Hendler.cpp
  src/FHT/Common/Controller/Logger/Logger.cpp
  src/FHT/Common/Controller/Task/Task.cpp
  src/FHT/Common/Controller/Client/Client.cpp
  src/FHT/Common/DBFacade/DBFacade.cpp
  src/FHT/Config/Config.cpp
  ${DBCPP}
)

if(WITH_COROUTINE)
  add_definitions(-DWITH_COROUTINE)
  set(SOURCE_LIB 
    ${SOURCE_LIB}
    src/FHT/Common/Controller/Server/ServerCoro.cpp
    src/FHT/Common/Controller/Client/WebClientCoro.cpp
  )
else()
  set(SOURCE_LIB 
    ${SOURCE_LIB}
    src/FHT/Common/Controller/Server/ServerAsync.cpp
    src/FHT/Common/Controller/Client/WebClientAsync.cpp
  )
endif()
set(HEADERS 
  "include"
  ${OPENSSL_INCLUDE_DIR}
  ${PostgreSQL_INCLUDE_DIRS})
add_library(fht STATIC ${SOURCE_LIB})
add_dependencies(fht
  Boost 
)
include_directories(${HEADERS})
set(FHT_LIBRARIES 
  fht
  ${PostgreSQL_LIBRARIES}
  ${OPENSSL_LIBRARIES}
  ${BOOST_LIBRARIES}
  ${pthread})
if(${DBCOMPILE} STREQUAL "Postgresql")
if(NOT MSVC)
  set(FHT_LIBRARIES 
    ${FHT_LIBRARIES}
    pq
  )
endif()
endif()

set(LINKER
  ${BINARY_DIR}/${BUILD_TYPE}
  ${PostgreSQL_LIBRARY_DIRS})
link_directories(${LINKER})
if(SAMPLE_BUILD)
  include(cmake/example.cmake)
endif()
