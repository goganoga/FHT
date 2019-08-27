# Framework Handler Task / FHT
Test ServerFrame
Default start service -> localhost:10800
# Build
- Linux (in directory)
 1. python build.py -b ninja
 or
 2. mk build && cd build && cmake .. -G "Unix Makefiles" && cmake build
 
 building binary -> ./build/bin
 
- Windows (in directory)
 1. build.bat -b ninja
 
 building binary -> ./build/Release
# Test
1.Testing the service from the browser.
  - You go to the test URL in browser: 
  ```bash
  http://localhost:10800/test?qq=qqq&xcdvdf=dsd
  ```
2.Testing the service from the curl.
  - You go to the test URL in curl: 
  ```bash
  curl -i -X POST -d "username=admin&password=admin&submit=Login" http://localhost:10800/test?qq=qqq&xcdvdf=dsd
  ```