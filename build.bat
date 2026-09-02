@echo off
REM ============================================================================
REM Build Script for Vehicle Rental Centre Management System
REM ============================================================================

echo [BUILD] Detecting C Compiler...

where gcc >nul 2>nul
if %ERRORLEVEL% EQU 0 (
    echo [BUILD] GCC detected. Compiling main.c...
    gcc -std=c99 -Wall -Wextra -O2 main.c -o vehicle_rental_centre.exe
    if %ERRORLEVEL% EQU 0 (
        echo [SUCCESS] Compilation successful: vehicle_rental_centre.exe
        echo Launching software...
        vehicle_rental_centre.exe
    ) else (
        echo [ERROR] Compilation failed.
    )
    goto end
)

where clang >nul 2>nul
if %ERRORLEVEL% EQU 0 (
    echo [BUILD] Clang detected. Compiling main.c...
    clang -std=c99 -Wall -Wextra -O2 main.c -o vehicle_rental_centre.exe
    if %ERRORLEVEL% EQU 0 (
        echo [SUCCESS] Compilation successful: vehicle_rental_centre.exe
        echo Launching software...
        vehicle_rental_centre.exe
    ) else (
        echo [ERROR] Compilation failed.
    )
    goto end
)

where cl >nul 2>nul
if %ERRORLEVEL% EQU 0 (
    echo [BUILD] MSVC CL compiler detected. Compiling main.c...
    cl /W3 /O2 /Fe:vehicle_rental_centre.exe main.c
    if %ERRORLEVEL% EQU 0 (
        echo [SUCCESS] Compilation successful: vehicle_rental_centre.exe
        echo Launching software...
        vehicle_rental_centre.exe
    ) else (
        echo [ERROR] Compilation failed.
    )
    goto end
)

where tcc >nul 2>nul
if %ERRORLEVEL% EQU 0 (
    echo [BUILD] Tiny C Compiler (TCC) detected. Compiling main.c...
    tcc main.c -o vehicle_rental_centre.exe
    if %ERRORLEVEL% EQU 0 (
        echo [SUCCESS] Compilation successful: vehicle_rental_centre.exe
        echo Launching software...
        vehicle_rental_centre.exe
    ) else (
        echo [ERROR] Compilation failed.
    )
    goto end
)

echo [!] No C compiler detected on system PATH.
echo [!] Please install MinGW-w64 (GCC), Clang, Visual Studio C++ (cl), or TCC.
echo [!] Once installed, compile using: gcc main.c -o vehicle_rental_centre.exe

:end
pause
