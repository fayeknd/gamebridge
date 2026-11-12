cmake_minimum_required(VERSION 3.24) 
project(drawbridge VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)

if (WIN32)
    message(NOTICE "Platform is WIN32.")
    set(CMAKE_EXE_LINKER_FLAGS "-static-libgcc -static-libstdc++ -lwinpthread -ldwmapi")
elseif(LINUX)
    message(NOTICE "Platform is LINUX.")
    set(CMAKE_EXE_LINKER_FLAGS "-static-libgcc -static-libstdc++")
else()
    message(NOTICE "Platform is unknown. Program will not function.")
endif()

# i fucking hate working with cmake shit but it's also so useful

set(FETCHCONTENT_QUIET OFF)
include(FetchContent)
set(BUILD_SHARED_LIBS OFF CACHE INTERNAL "") 

message(NOTICE "Fetching and building GLFW...")

set(GLFW_BUILD_EXAMPLES OFF CACHE INTERNAL "")
set(GLFW_BUILD_TESTS OFF CACHE INTERNAL "")
set(GLFW_BUILD_DOCS OFF CACHE INTERNAL "")
FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw
    GIT_TAG        3.5.1
    GIT_PROGRESS   TRUE
)
FetchContent_MakeAvailable(glfw)

# for c++26
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
#set(BOOST_INCLUDE "")


message(NOTICE "Fetching and building zlib...")

# build zlib 

set(ZLIB_INCLUDE_DIRS "${zlib_SOURCE_DIR};${zlib_BINARY_DIR}" CACHE INTERNAL "")
set(ZLIB_INCLUDE_DIR "${zlib_SOURCE_DIR};${zlib_BINARY_DIR}" CACHE INTERNAL "")
set(ZLIB_BUILD_SHARED OFF CACHE BOOL "disable zlib shared library build" FORCE)
set(ZLIB_BUILD_STATIC ON CACHE BOOL "enable zlib static library build" FORCE)
set(ZLIB_BUILD_EXAMPLES OFF CACHE BOOL "disable zlib examples" FORCE)


FetchContent_Declare(
    zlib
    GIT_REPOSITORY https://github.com/madler/zlib
    GIT_TAG        v1.3.2
    GIT_PROGRESS   TRUE
)
FetchContent_MakeAvailable(zlib)

# lie to cmake and say we found zlib
set(ZLIB_FOUND TRUE CACHE INTERNAL "")

if(TARGET zlibstatic)
    if(NOT TARGET ZLIB::ZLIB)
        add_library(ZLIB::ZLIB ALIAS zlibstatic)
    endif()
    set(ZLIB_LIBRARIES zlibstatic CACHE INTERNAL "")
endif()

message(NOTICE "Fetching and building Assimp (ass pimp)...")

set(ASSIMP_WARNINGS_AS_ERRORS OFF CACHE INTERNAL "assimp has unused variables, don't treat them as errors or we won't build")
set(ASSIMP_SYSTEM_ZLIB ON CACHE INTERNAL "use our zlib instead")
set(ASSIMP_INSTALL OFF CACHE INTERNAL "turn off assimp install targets")
set(ASSIMP_BUILD_ZLIB OFF CACHE INTERNAL "don't make assimp build it's own zlib")

FetchContent_Declare(
    assimp
    GIT_REPOSITORY https://github.com/assimp/assimp
    GIT_TAG        v6.0.5
    GIT_PROGRESS   TRUE
)
FetchContent_MakeAvailable(assimp)
if(TARGET assimp)
    target_include_directories(assimp PRIVATE 
        "${zlib_SOURCE_DIR}"
        "${zlib_SOURCE_DIR}/contrib/minizip"
    )
endif()


message(NOTICE "Fetching and building libzip...")

# christ
set(ENABLE_ZSTD OFF CACHE INTERNAL "disable zstd support")
set(ENABLE_LZMA OFF CACHE INTERNAL "disable lzma support")
set(ENABLE_BZIP2 OFF CACHE INTERNAL "disable bzip2 support")
set(LIBZIP_DO_INSTALL OFF CACHE INTERNAL "")
set(BUILD_DOC OFF CACHE INTERNAL "")
set(BUILD_EXAMPLES OFF CACHE INTERNAL "")
set(BUILD_REGRESS OFF CACHE INTERNAL "")

# build libzip

FetchContent_Declare(
    libzip
    GIT_REPOSITORY https://github.com/nih-at/libzip
    GIT_TAG        v1.12.1
    GIT_PROGRESS   TRUE
)
FetchContent_MakeAvailable(libzip)
 

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

list(REMOVE_ITEM REFLECTABLE_HEADERS "${GENERATED_HEADER}")

find_package(Python3 REQUIRED COMPONENTS Interpreter)

add_custom_command(
    OUTPUT ${GENERATED_HEADER}
    COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/codegen.py ${GENERATED_HEADER} ${REFLECTABLE_HEADERS}
    DEPENDS ${REFLECTABLE_HEADERS} ${CMAKE_CURRENT_SOURCE_DIR}/tools/codegen.py
    COMMENT "Generating reflection header map..."
    VERBATIM
)

add_custom_target(reflect_codegen DEPENDS ${GENERATED_HEADER})

# configure what we're linking

file(GLOB_RECURSE ENGINE_SOURCES CONFIGURE_DEPENDS 
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/data/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/data/*.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/*.c"
)

file(GLOB_RECURSE PROJECT_HEADERS CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/*.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/data/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/data/*.hpp"
)

# editor lib
file(GLOB_RECURSE EDITOR_SOURCES CONFIGURE_DEPENDS 
    "${CMAKE_CURRENT_SOURCE_DIR}/editor/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/editor/*.hpp"
)

set(ENGINE_TARGET_INCLUDES 
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/data
    ${CMAKE_CURRENT_SOURCE_DIR}/data/utils
    ${CMAKE_CURRENT_SOURCE_DIR}/data/generated
    ${BOOST_INCLUDE}
)

set(EDITOR_TARGET_INCLUDES
    ${CMAKE_CURRENT_SOURCE_DIR}/editor
)

set(TARGET_LIBS glfw zip assimp)

#add_compile_options(-O1 -g -fsanitize=address -fno-omit-frame-pointer)
add_compile_options(-O3 -fno-omit-frame-pointer)
#add_link_options(-fsanitize=address) # more detailed memory leak and segfault errors

# core engine lib
add_library(engine ${PROJECT_HEADERS} ${ENGINE_SOURCES})
add_dependencies(engine resources reflect_codegen)
target_link_libraries(engine PUBLIC ${TARGET_LIBS})
target_include_directories(engine PUBLIC ${ENGINE_TARGET_INCLUDES})

# editor binary
add_executable(editor ${EDITOR_SOURCES})
target_link_libraries(editor PRIVATE engine)
target_include_directories(editor PUBLIC ${EDITOR_TARGET_INCLUDES})


include(CTest)
enable_testing()

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)

#if (COMMAND copy_dlls)

    #copy_dlls(engine)

#endif()