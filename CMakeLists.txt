cmake_minimum_required(VERSION 3.13)

project(sqlite-starter-cpp)

set(CMAKE_CXX_STANDARD 23) # Enable the C++23 standard

file(GLOB_RECURSE SOURCE_FILES src/*.cpp include/*.hpp)

add_executable(exe ${SOURCE_FILES})
target_include_directories(exe PRIVATE include)