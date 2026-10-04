cmake_minimum_required(VERSION 3.10.0)

set(CMAKE_SYSTEM_NAME Windows)
set(CMAKE_SYSTEM_PROCESSOR x86_64)

set(CMAKE_C_COMPILER "clang;--target=x86_64-w64-windows-gnu;-fuse-ld=lld")
set(CMAKE_CXX_COMPILER "clang++;--target=x86_64-w64-windows-gnu;-fuse-ld=lld")

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

project(gamebridge VERSION 0.1.0 LANGUAGES C CXX)

# add any folders with source files here
file(GLOB_RECURSE PROJECT_SOURCES CONFIGURE_DEPENDS 
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/*.hpp"
)

add_executable(gamebridge ${PROJECT_SOURCES})

include(CTest)
enable_testing()

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
