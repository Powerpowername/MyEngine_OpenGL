cmake_minimum_required(VERSION 3.27)

project(Myproject VERSION 1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/build/bin)
# 非递归查找
file(GLOB SOURCE "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c")
message(STATUS "adsdf ${HEADERS}")

file(GLOB TEMP "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")
list(APPEND SOURCE ${TEMP})
message(STATUS "adsdf ${SOURCE}")
add_executable(app main.cpp)

target_include_directories(app PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_sources(app PUBLIC ${SOURCE})

# find_package(OpenGL REQUIRED)
target_link_libraries(app PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}/lib/libglfw3.a
    ${CMAKE_CURRENT_SOURCE_DIR}/build/bin/libassimp-5.dll)

