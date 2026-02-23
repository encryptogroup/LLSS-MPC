cmake_minimum_required(VERSION 3.15...3.30)
project(
  DelayedresharingOptimizer
  VERSION 1.0
  LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)

if(CMAKE_BUILD_TYPE STREQUAL "DEBUG")
    add_definitions(-DDEBUG) 
endif()

include_directories(.)


include_directories(../delayedresharing)
link_directories(../delayedresharing/build)

add_executable(DelayedResharing main_optimizer.cpp)

target_link_libraries(DelayedResharing delayedresharing generalized_terms optimizations cost_metrics)
