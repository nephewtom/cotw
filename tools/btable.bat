:: echo off
del gentable.exe

:: gcc -g -Wall -pedantic -c ../sds/sds.c
gcc -g -Wall -I. -I.. gentable.cpp -o gentable.exe sds.o

.\gentable.exe
