# CMakeList.txt : CMake project for DWM, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug)
endif()

set(CMAKE_C_COMPILER "gcc")
set(CMAKE_CXX_COMPILER "g++")
set(CMAKE_C_FLAGS "-Wall -Wextra -lusb-1.0")
set(CMAKE_C_FLAGS_DEBUG "-g")
set(CMAKE_C_FLAGS_RELEASE "-O3")

# Add source to this project's executable.
add_executable (usb-test "usb-test.c" "pl25a1.h")
target_link_libraries(usb-test usb-1.0)
