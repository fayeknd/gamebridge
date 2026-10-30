cmake_minimum_required(VERSION 3.24) 
set(CMAKE_CXX_STANDARD 20)

# i fucking hate working with cmake shit but it's also so useful

set(FETCHCONTENT_QUIET OFF)
include(FetchContent)

message(NOTICE "Fetching and building GLFW...")

FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw
    GIT_TAG        3.5.1
    GIT_PROGRESS   TRUE
)
set(GLFW_BUILD_EXAMPLES OFF CACHE INTERNAL "")
set(GLFW_BUILD_TESTS OFF CACHE INTERNAL "")
set(GLFW_BUILD_DOCS OFF CACHE INTERNAL "")
FetchContent_MakeAvailable(glfw)

#message(NOTICE "Fetching and building Boost PFR Reflection module...")
#
#FetchContent_Declare(
#    boost_pfr
#    GIT_REPOSITORY https://github.com/boostorg/pfr
#    GIT_TAG        boost-1.92.0 
#    GIT_SHALLOW    TRUE # only pull the single commit
#    GIT_PROGRESS   TRUE
#)
#set(BUILD_TESTING OFF CACHE INTERNAL "disable testing for sub-dependencies")
#FetchContent_MakeAvailable(boost_pfr)
#set(BOOST_INCLUDE ${boost_pfr_SOURCE_DIR}/include)
set(BOOST_INCLUDE "")


# libzip is tricky on windows so fuck you build it from source
if(WIN32)

    message(NOTICE "Platform is WIN32")
    message(NOTICE "Fetching and building zlib and libzip...")

    set(CMAKE_EXE_LINKER_FLAGS "-static-libgcc -static-libstdc++ -lwinpthread -ldwmapi -static")

    # build zlib 
    FetchContent_Declare(
        zlib
        GIT_REPOSITORY https://github.com/madler/zlib
        GIT_TAG        v1.3.2
        GIT_PROGRESS   TRUE
    )
    set(BUILD_SHARED_LIBS OFF CACHE INTERNAL "") 
    FetchContent_MakeAvailable(zlib)

    # lie to cmake and say we found zlib
    set(ZLIB_FOUND TRUE CACHE INTERNAL "")

    if(TARGET zlibstatic)
        if(NOT TARGET ZLIB::ZLIB)
            add_library(ZLIB::ZLIB ALIAS zlibstatic)
        endif()
        set(ZLIB_LIBRARIES zlibstatic CACHE INTERNAL "")
    endif()

    set(ZLIB_INCLUDE_DIRS "${zlib_SOURCE_DIR};${zlib_BINARY_DIR}" CACHE INTERNAL "")
    set(ZLIB_INCLUDE_DIR "${zlib_SOURCE_DIR};${zlib_BINARY_DIR}" CACHE INTERNAL "")

    # build libzip
    FetchContent_Declare(
        libzip
        GIT_REPOSITORY https://github.com/nih-at/libzip
        GIT_TAG        v1.12.1
        GIT_PROGRESS   TRUE
    )
    set(LIBZIP_DO_INSTALL OFF CACHE INTERNAL "")
    set(BUILD_DOC OFF CACHE INTERNAL "")
    set(BUILD_EXAMPLES OFF CACHE INTERNAL "")
    set(BUILD_REGRESS OFF CACHE INTERNAL "")
    set(BUILD_SHARED_LIBS OFF CACHE INTERNAL "")
    FetchContent_MakeAvailable(libzip)

else()
    message(NOTICE "Platform is Linux. Assuming libzip/zlib is already installed.")
endif()


# resource packing
# rsc_source is /resources/ in the project directory
# rsc_out is the output. (in the binary directory.)

set(RSC_FILE "stuff")

set(RSC_SOURCE "${CMAKE_CURRENT_SOURCE_DIR}/resources")
set(RSC_TARGET "${CMAKE_CURRENT_BINARY_DIR}/${RSC_FILE}")
file(GLOB_RECURSE RESOURCES_OUT CONFIGURE_DEPENDS "${RSC_SOURCE}/*")

add_custom_target(resources
    COMMAND ${CMAKE_COMMAND} -E tar "cf" "${RSC_TARGET}" --format=zip "."
    WORKING_DIRECTORY "${RSC_SOURCE}"
    COMMENT "Compressing project resources into '${RSC_FILE}' archive..."
    VERBATIM
) 

# just the headers
file(GLOB_RECURSE REFLECTABLE_HEADERS CONFIGURE_DEPENDS 
    "${CMAKE_CURRENT_SOURCE_DIR}/data/component/*.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/data/*.hpp"
)

set(GENERATED_HEADER "${CMAKE_CURRENT_SOURCE_DIR}/data/generated/generated_reflection.hpp")

add_custom_command(
    OUTPUT ${GENERATED_HEADER}
    COMMAND python3 ${CMAKE_CURRENT_SOURCE_DIR}/tools/codegen.py ${GENERATED_HEADER} ${REFLECTABLE_HEADERS}
    DEPENDS ${COMPONENT_HEADERS} ${CMAKE_CURRENT_SOURCE_DIR}/tools/codegen.py
    COMMENT "Generating reflection header map..."
    VERBATIM
)

add_custom_target(reflect_codegen DEPENDS ${GENERATED_HEADER})

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

    "${CMAKE_CURRENT_SOURCE_DIR}/data/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/data/*.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/data/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/data/*.hpp"
)


add_executable(gamebridge ${PROJECT_SOURCES})
add_compile_options(gamebridge -O0 -freflection-latest)
add_dependencies(gamebridge resources reflect_codegen)

#debug stuff
#target_compile_options(gamebridge PRIVATE -fsanitize=address,undefined -g)
#target_link_libraries(gamebridge PRIVATE -fsanitize=address,undefined)


# haha lol i dont have to build libzip from source fuuuuckkkk yooouuuu
if (NOT WIN32)
    find_package(libzip CONFIG REQUIRED)
    target_link_libraries(gamebridge PRIVATE glfw libzip::zip)
else()
    target_link_libraries(gamebridge PRIVATE glfw zip)
endif()

include(CTest)
enable_testing()

#include dirs
target_include_directories(gamebridge PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/data
    ${CMAKE_CURRENT_SOURCE_DIR}/data/utils
    ${CMAKE_CURRENT_SOURCE_DIR}/data/generated
    BOOST_INCLUDE
)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)

if (COMMAND copy_dlls)

    copy_dlls(gamebridge)

endif()