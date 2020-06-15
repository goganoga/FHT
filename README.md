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
How to do this is described in the "simple" examples.
Just take a look at them.
It's not hard :-)