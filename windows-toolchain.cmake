set(CMAKE_SYSTEM_NAME Windows)
set(CMAKE_SYSTEM_PROCESSOR x86_64)

set(CMAKE_C_COMPILER x86_64-w64-mingw32-gcc)
set(CMAKE_CXX_COMPILER x86_64-w64-mingw32-g++)
set(CMAKE_RC_COMPILER x86_64-w64-mingw32-windres)

set(CMAKE_FIND_ROOT_PATH /usr/x86_64-w64-mingw32)
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

set(CMAKE_EXE_LINKER_FLAGS "-static-libgcc -static-libstdc++ -lwinpthread -ldwmapi" CACHE STRING "" FORCE)

function(copy_dlls target)
    
    # if compiling on an actual windows computer
    if(CMAKE_HOST_SYSTEM_NAME STREQUAL "Windows")
        add_custom_command(
            TARGET ${target} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                $<TARGET_RUNTIME_DLLS:${target}>
                $<TARGET_FILE_DIR:${target}>
            COMMAND_EXPAND_LISTS
            COMMENT "Trying to find Windows DLLs..."
        )

        # try find mingw's libstdc++.dll
        execute_process(
            COMMAND ${CMAKE_CXX_COMPILER} -print-file-name=libstdc++-6.dll
            OUTPUT_VARIABLE COMPILER_STDCPP_PATH
            OUTPUT_STRIP_TRAILING_WHITESPACE
        )
        get_filename_component(COMPILER_BIN_DIR "${COMPILER_STDCPP_PATH}" DIRECTORY)

        if(EXISTS "${COMPILER_BIN_DIR}")
            add_custom_command(
                TARGET ${target} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    "${MINGW_BIN_DIR}/libgcc_s_seh-1.dll"
                    "${MINGW_BIN_DIR}/libstdc++-6.dll"
                    "${MINGW_BIN_DIR}/libzip.dll"
                    "${MINGW_BIN_DIR}/libssp-0.dll"
                    "${MINGW_BIN_DIR}/libbz2-1.dll"
                    "${MINGW_BIN_DIR}/liblzma-5.dll"
                    "${MINGW_BIN_DIR}/zlib1.dll"
                    "${MINGW_BIN_DIR}/libwinpthread-1.dll"
                    "$<TARGET_FILE_DIR:${target}>"
                COMMENT "Copying DLLs to build folder..."
            )
        endif()

    # else, on linux
    else()

        # the three places mingw can be.
        set(LINUX_MINGW_PATHS
            "/usr/x86_64-w64-mingw32/bin" # arch
            "/usr/x86_64-w64-mingw32/sys-root/mingw/bin"
            "/usr/w64-mingw32/bin"
        )

        # our stupid dlls
        set(REQUIRED_DLLS
            "libgcc_s_seh-1.dll"
            "libstdc++-6.dll"
            "libzip.dll"
            "libssp-0.dll"
            "libbz2-1.dll"
            "liblzma-5.dll"
            "zlib1.dll"
            "libwinpthread-1.dll"
        )

        foreach(dll ${REQUIRED_DLLS})
            # try find the mingw dlls
            find_file(TOOLCHAIN_DLL_${dll} NAMES ${dll} PATHS ${LINUX_MINGW_PATHS} NO_DEFAULT_PATH)
            
            if(TOOLCHAIN_DLL_${dll})
                add_custom_command(
                    TARGET ${target} POST_BUILD
                    COMMAND ${CMAKE_COMMAND} -E copy_if_different
                        "${TOOLCHAIN_DLL_${dll}}"
                        "$<TARGET_FILE_DIR:${target}>"
                    COMMENT "Copying DLLs to build folder..."
                )
            endif()
            unset(TOOLCHAIN_DLL_${dll} CACHE)
        endforeach()
    endif()
endfunction()