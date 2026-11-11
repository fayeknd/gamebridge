@echo off
echo Building for Windows, then running...

if "%~1"=="-fresh" (
    echo Fresh build! This might take a minute...
    if exist compile_commands.json del compile_commands.json
    if exist build\windows rmdir /s /q build\windows
    if "%~2"=="-release" (
        cmake -B build/windows -G "Ninja" -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DCMAKE_TOOLCHAIN_FILE="%CD%/windows-toolchain.cmake" --fresh -DCMAKE_BUILD_TYPE=MinSizeRel
    ) else (
        cmake -B build/windows -G "Ninja" -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DCMAKE_TOOLCHAIN_FILE="%CD%/windows-toolchain.cmake" --fresh
    )
) else (
    if "%~2"=="-release" (
        cmake -B build/windows -G "Ninja" -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DCMAKE_TOOLCHAIN_FILE="%CD%/windows-toolchain.cmake" -DCMAKE_BUILD_TYPE=MinSizeRel
    ) else (
        cmake -B build/windows -G "Ninja" -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DCMAKE_TOOLCHAIN_FILE="%CD%/windows-toolchain.cmake"
    )
)

if %ERRORLEVEL% NEQ 0 (
    echo CMake configuration failed.
    exit /b %ERRORLEVEL%
)

if not exist compile_commands.json (
    if exist build\windows\compile_commands.json (
        mklink compile_commands.json build\windows\compile_commands.json
    )
)

cmake --build build/windows
if %ERRORLEVEL% NEQ 0 (
    echo Build failed.
    exit /b %ERRORLEVEL%
)

echo Launching executable...
build\windows\gamebridge.exe