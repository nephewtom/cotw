@echo off
del .\cotw.exe

:: Set you raylib installation path
set RAYLIB_DIR=..\rolling-cube\raylib\src
@echo on
g++ -g -Wall cotw.cpp -o cotw.exe -I%RAYLIB_DIR% -L%RAYLIB_DIR% -lraylib -lgdi32 -lwinmm -std=c++20
@echo off
.\cotw.exe
