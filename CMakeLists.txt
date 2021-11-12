cmake_minimum_required(VERSION 3.13)

project(HClustering)

file(GLOB_RECURSE SOURCES "src/*.cpp" "src/KaHIP/*.cpp" main.cc)
add_executable(${PROJECT_NAME} ${SOURCES})

target_include_directories(${PROJECT_NAME} PUBLIC include)


# xgboost library for ML
add_subdirectory(xgboost)
target_link_libraries(${PROJECT_NAME} xgboost)


# graph data structures and functionalities
#add_subdirectory(HGraph)
#target_include_directories(${PROJECT_NAME} PUBLIC HGraph/include)
#target_link_libraries(${PROJECT_NAME} HGraph)