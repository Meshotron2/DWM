# CMakeList.txt : CMake project for DWM, include source and define
# project specific logic here.
# https://stackoverflow.com/questions/41361631/optimize-in-cmake-by-default

cmake_minimum_required(VERSION 3.8)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug)
endif()

set(CMAKE_C_COMPILER "mpicc")
set(CMAKE_CXX_COMPILER "mpic++")
set(CMAKE_C_FLAGS "-Wall -Wextra -pthread")
set(CMAKE_C_FLAGS_DEBUG "-g")
set(CMAKE_C_FLAGS_RELEASE "-O3")

# Add source to this project's executable.
add_executable(mpi "main.c" "main.h" "facebuffer.c" "facebuffer.h" "header.h" "node.c" "node.h" "config.c" "config.h" "communicator.c" "communicator.h")

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  ADD_LIBRARY(math STATIC IMPORTED)
  target_link_libraries(mpi m)
endif()