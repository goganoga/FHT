cmake_minimum_required(VERSION 2.8)

project(FHT CXX)
set (EXECUTABLE_OUTPUT_PATH bin)
if(NOT MSVC)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")
endif()
if(MSVC)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++17")
endif()
set(SOURCE_EXE FHT/TestFHT.cpp)
set(SOURCE_LIB 
  FHT/Hendler.cpp
  FHT/Task.cpp)
set(HEADER 
  FHT/Controller.h
  FHT/iController.h
  FHT/Hendler.h
  FHT/iHendler.h
  FHT/Task.h 
  FHT/iTask.h)
add_library(FHT_LIB STATIC ${SOURCE_LIB})
add_executable(TestFHT ${SOURCE_EXE})
target_link_libraries(TestFHT FHT_LIB)
