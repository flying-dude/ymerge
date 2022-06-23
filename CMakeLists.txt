cmake_minimum_required(VERSION 3.5)

project(ymerge)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(fmt)

include_directories(src/youtil)
include_directories(src/ymerge)

file(GLOB_RECURSE youtil_FILES src/youtil/*.cpp src/youtil/*.c)
add_library(youtil SHARED ${youtil_FILES})

file(GLOB_RECURSE ymerge_FILES src/ymerge/*.cpp)
add_executable(ymerge ${ymerge_FILES})
target_link_libraries(ymerge youtil fmt::fmt)
