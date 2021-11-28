cmake_minimum_required(VERSION 3.13)

project(PGD)

file(GLOB_RECURSE SOURCES "source/*.cpp")
add_library(${PROJECT_NAME} ${SOURCES})

set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ffast-math -funroll-loops -fno-strict-aliasing \
-fomit-frame-pointer -fexpensive-optimizations -funroll-loops \
-fmove-loop-invariants -fprefetch-loop-arrays -ftree-loop-optimize \
-ftree-vect-loop-version -ftree-vectorize")

target_include_directories(${PROJECT_NAME} PUBLIC header)