# https://stackoverflow.com/questions/41361631/optimize-in-cmake-by-default

cmake_minimum_required(VERSION 3.0.0)
project(DWM VERSION 0.1.0)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug)
endif()

set(CMAKE_C_COMPILER "mpicc")
set(CMAKE_CXX_COMPILER "mpic++")
set(CMAKE_C_FLAGS "-Wall -Wextra")
set(CMAKE_C_FLAGS_DEBUG "-g")
set(CMAKE_C_FLAGS_RELEASE "-O3")

add_executable(mpi "main.c" "main.h" "facebuffer.c" "facebuffer.h" "header.h" "node.c" "node.h" "config.c" "config.h")
ADD_LIBRARY(math STATIC IMPORTED)
target_link_libraries(mpi m)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
