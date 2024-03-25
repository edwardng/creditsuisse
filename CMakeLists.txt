cmake_minimum_required(VERSION 3.23)
project(MatchingEngine LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_EXPORT_COMPILE_COMMANDS "ON")

include_directories(lib/include)

set(Boost_USE_STATIC_LIBS OFF)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME OFF)
find_package(Boost 1.74.0 COMPONENTS thread unit_test_framework REQUIRED)

set(ME_LIB_SOURCE
        lib/src/include.cpp
        lib/src/engine/default_engine_event_handler.cpp)

add_library(ME_LIB ${ME_LIB_SOURCE})
set_target_properties(ME_LIB PROPERTIES LINKER_LANGUAGE CXX)

add_subdirectory(test)
