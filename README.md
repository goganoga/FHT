# Framework Handler Task / FHT
Framework Handler and Task manager as a skeleton for web service. Written in C++17
- Templates - https://github.com/goganoga/FHT/tree/master/src/TemplatesFHT

# This frame contains:
  - HandlerManager (Need for call a 'callback' from anywhere in the program)
  - TaskManager (Sheduler and loop sheduler as a kernel in the program. Realization multithread)
  - WebService (Realization a network worker to easily develop Internet services. Using libevent - https://github.com/libevent/libevent.git)
  - WebClient (Realization a network worker to get or post request http(s). Using libevent - https://github.com/libevent/libevent.git)
# Build
- Linux (in directory) variant
   - python build.py -b ninja
   - mk build && cd build && cmake .. -G "Unix Makefiles" && cmake build
 building library -> ./build/bin
 
- Windows (in directory)
   - build.bat -b ninja
 building library -> ./build/Release
 
# Include to the project:
You need include to your cmake file, if you want to use it:
```bash
  if(MSVC)
    set(LIB_FOR_WIN 
    iphlpapi
    shell32
    advapi32
    ws2_32
    Crypt32)
    set(BUILD_TYPE ${CMAKE_BUILD_TYPE})
  endif()
  if(NOT MSVC)
    set(pthread event_pthreads)
  endif()
  find_package(OpenSSL REQUIRED)
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
  include_directories(
    ${SOURCE_DIR}/src/FHT/Interfice
    ${OPENSSL_INCLUDE_DIR})
  link_directories(${LINKER})
  add_dependencies(${YourTarget} fhtlib)
  target_link_libraries(${YourTarget} 
    ${LIB_FOR_WIN}
    ${FHT_LIBRARIES})
```
