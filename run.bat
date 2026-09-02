@echo off
title Apex Auto Wheels - Vehicle Rental System (C Software)
cd /d "%~dp0"
if not exist vehicle_rental_centre.exe (
    echo [BUILD] Compiling C source code...
    gcc -std=c99 -Wall -Wextra -O2 main.c -o vehicle_rental_centre.exe
)
vehicle_rental_centre.exe
pause
