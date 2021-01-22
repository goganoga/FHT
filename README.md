# Framework Handler Task / FHT
Framework Handler and Task manager as a skeleton for web service. Implemented on C++17
- Templates - https://github.com/goganoga/FHT/tree/master/src/Sample

# This frame contains:
  - HandlerManager (Need for call a 'callback' from anywhere in the program)
  - TaskManager (Sheduler and loop sheduler as a kernel in the program. Realization multithread)
  - WebService (Realization a network worker to easily develop Internet services. Using libevent - https://github.com/libevent/libevent.git)
  - WebClient (Realization a network worker to get or post request http(s). Using libevent - https://github.com/libevent/libevent.git)
  - DbFacade (Connector to database)
# Build
- Linux (in directory) variant
   - python build.py -b ninja
 building library -> ./build/bin
 
- Windows (in directory)
   - build.bat -b ninja
 building library -> ./build/Release
 
# Include to the project:
```cmake
list(APPEND CMAKE_ARGS_FHT 
      -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
      -DEXAMPLE_BUILD=OFF)
ExternalProject_Add(fhtlib
    BINARY_DIR fhtlib/build
    PREFIX fhtlib
    GIT_REPOSITORY https://github.com/goganoga/FHT.git
    TIMEOUT 10
    UPDATE_COMMAND ${GIT_EXECUTABLE} pull
    CMAKE_ARGS ${CMAKE_ARGS_FHT}
    INSTALL_COMMAND ""
    LOG_DOWNLOAD ON
)
ExternalProject_Get_property(fhtlib BINARY_DIR SOURCE_DIR)
```
Just take a look at them.
It's not hard :-)