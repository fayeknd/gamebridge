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

# resource packing
# rsc_source is /resources/ in the project directory
# rsc_out is the output. (in the binary directory.)

set(RSC_FILE "resources")

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

if(WIN32)
    if(MINGW)
        set(CMAKE_EXE_LINKER_FLAGS "-static -static-libgcc -static-libstdc++ -lwinpthread -ldwmapi")
    endif()

elseif(UNIX AND NOT APPLE)
    set(CMAKE_EXE_LINKER_FLAGS "")
endif()

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
find_package(libzip REQUIRED)
target_link_libraries(gamebridge PRIVATE glfw libzip::zip)

include(CTest)
enable_testing()

target_include_directories(gamebridge PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
