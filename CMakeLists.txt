cmake_minimum_required(VERSION 3.10.0)

set(CMAKE_SYSTEM_NAME Windows)
set(CMAKE_SYSTEM_PROCESSOR x86_64)
set(CMAKE_C_COMPILER clang)
set(CMAKE_CXX_COMPILER clang++)

set(FLAGS "
    -static
    -static-libgcc
    -static-libstdc++
    -lwinpthread
    -l/lib/glfw3
")

if(UNIX)
    set(CMAKE_C_FLAGS "-target x86_64-w64-windows-gnu -fuse-ld=lld")
    set(CMAKE_CXX_FLAGS "-target x86_64-w64-windows-gnu -fuse-ld=lld")
    set(CMAKE_EXE_LINKER_FLAGS "-static -static-libgcc -static-libstdc++ -lwinpthread")
endif()

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


add_custom_command(TARGET gamebridge POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${CMAKE_CURRENT_SOURCE_DIR}/lib/glfw3.dll"
    $<TARGET_FILE_DIR:gamebridge>
)
