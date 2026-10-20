cmake_minimum_required(VERSION 3.10.0)
set(CMAKE_CXX_STANDARD 20)

# i fucking hate working with cmake shit but it's also so useful

include(FetchContent)
FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw
    GIT_TAG        3.5.1
)
set(GLFW_BUILD_EXAMPLES OFF CACHE INTERNAL "")
set(GLFW_BUILD_TESTS OFF CACHE INTERNAL "")
set(GLFW_BUILD_DOCS OFF CACHE INTERNAL "")
FetchContent_MakeAvailable(glfw)

# libzip is tricky on windows so fuck you build it from source
if(WIN32)

    set(CMAKE_EXE_LINKER_FLAGS "-static-libgcc -static-libstdc++ -lwinpthread -ldwmapi -static")

    FetchContent_Declare(
        zlib
        GIT_REPOSITORY https://github.com/madler/zlib
        GIT_TAG        v1.3.2
    )
    set(BUILD_SHARED_LIBS OFF CACHE INTERNAL "") 
    FetchContent_MakeAvailable(zlib)

    if(NOT TARGET ZLIB::ZLIB AND TARGET zlibstatic)
        add_library(ZLIB::ZLIB ALIAS zlibstatic)
    endif()


    FetchContent_Declare(
        libzip
        GIT_REPOSITORY https://github.com/nih-at/libzip
        GIT_TAG        v1.12.1 
    )
    set(LIBZIP_DO_INSTALL OFF CACHE INTERNAL "")
    set(BUILD_DOC OFF CACHE INTERNAL "")
    set(BUILD_EXAMPLES OFF CACHE INTERNAL "")
    set(BUILD_REGRESS OFF CACHE INTERNAL "")
    set(BUILD_SHARED_LIBS OFF CACHE INTERNAL "")
    FetchContent_MakeAvailable(libzip)

endif()


# resource packing
# rsc_source is /resources/ in the project directory
# rsc_out is the output. (in the binary directory.)

set(RSC_FILE "stuff")

set(RSC_SOURCE "${CMAKE_CURRENT_SOURCE_DIR}/resources")
set(RSC_TARGET "${CMAKE_CURRENT_BINARY_DIR}/${RSC_FILE}")
file(GLOB_RECURSE RESOURCES_OUT CONFIGURE_DEPENDS "${RSC_SOURCE}/*")

add_custom_command(
    OUTPUT "${RSC_TARGET}"
    COMMAND ${CMAKE_COMMAND} -E tar "cf" "${RSC_TARGET}" --format=zip "."
    WORKING_DIRECTORY "${RSC_SOURCE}"
    DEPENDS ${RESOURCES_OUT}
    COMMENT "Compressing project resources into '${RSC_FILE}' archive..."
)
add_custom_target(resources DEPENDS "${RSC_TARGET}")

project(gamebridge VERSION 0.1.0 LANGUAGES C CXX)
add_library(glad STATIC 
    "${CMAKE_CURRENT_SOURCE_DIR}/src/glad.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/glad/glad.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/KHR/khrplatform.h"
)
target_include_directories(glad PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")

# add any folders with source files here
file(GLOB_RECURSE PROJECT_SOURCES CONFIGURE_DEPENDS 
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.hpp"

    "${CMAKE_CURRENT_SOURCE_DIR}/include/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/*.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/*.hpp"
)

add_executable(gamebridge ${PROJECT_SOURCES})
add_dependencies(gamebridge resources)

# haha lol i dont have to build libzip from source fuuuuckkkk yooouuuu
if (NOT WIN32)
    find_package(libzip CONFIG REQUIRED)
    target_link_libraries(gamebridge PRIVATE glfw libzip::zip)
else()
    target_link_libraries(gamebridge PRIVATE glfw zip)
endif()

include(CTest)
enable_testing()

target_include_directories(gamebridge PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include ${CMAKE_CURRENT_SOURCE_DIR}/data)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)

if (COMMAND copy_dlls)

    copy_dlls(gamebridge)

endif()