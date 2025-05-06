del gen-table.exe

:: gcc -g -Wall -pedantic -I./sds -c sds/sds.c
g++ -g -Wall -I. -I./sds gen-table.cpp -o gen-table.exe sds.o

.\gen-table.exe
