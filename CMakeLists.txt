cmake_minimum_required(VERSION 3.5)

project(ymerge)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wp,-D_FORTIFY_SOURCE=2")

if(DEFINED YMERGE_VERSION)
    add_definitions(-DYMERGE_VERSION="${YMERGE_VERSION}")
else()
    add_definitions(-DYMERGE_VERSION="UNKNOWN_VERSION")
endif()

find_package(fmt)

include_directories(src/youtil)
include_directories(src/ymerge)

file(GLOB_RECURSE youtil_FILES src/youtil/*.cpp src/youtil/*.c)
add_library(youtil SHARED ${youtil_FILES})

file(GLOB_RECURSE ymerge_FILES src/ymerge/*.cpp)
add_executable(ymerge ${ymerge_FILES})
target_link_libraries(ymerge youtil fmt::fmt)
