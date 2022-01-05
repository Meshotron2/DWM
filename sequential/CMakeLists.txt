# CMakeList.txt : CMake project for DWM, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

# Add source to this project's executable.
add_executable (Cluster "cluster.c" "types.h" "cluster.h" "node.c" "node.h")

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    ADD_LIBRARY(math STATIC IMPORTED)
    target_link_libraries(Cluster m)
 endif()
