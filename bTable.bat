echo off
del gentable.exe

:: gcc -g -Wall -pedantic -I./sds -c sds/sds.c
gcc -g -Wall -I. -I./sds gentable.cpp -o gentable.exe sds.o

.\gentable.exe
