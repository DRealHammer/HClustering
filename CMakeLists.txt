cmake_minimum_required(VERSION 3.13)

project(HClustering)

file(GLOB_RECURSE SOURCES "src/*.cpp" "src/KaHIP/*.cpp" main.cc argtable/argtable3.c)
add_executable(${PROJECT_NAME} ${SOURCES})
find_package(OpenMP)
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS} -std=c++11")

target_include_directories(${PROJECT_NAME} PUBLIC include)


# xgboost library for ML
add_subdirectory(xgboost)
target_link_libraries(${PROJECT_NAME} xgboost)

# PGD for Graphlets
add_subdirectory(PGD)
target_link_libraries(${PROJECT_NAME} PGD)

